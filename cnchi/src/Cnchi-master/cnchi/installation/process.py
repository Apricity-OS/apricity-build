#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  process.py
#
#  Copyright © 2015 Apricity
#
#  This file is part of Cnchi.
#
#  Cnchi is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  Cnchi is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Cnchi; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.

""" Installation thread module. """

import crypt
import logging
import multiprocessing
import os
import queue
import shutil
import subprocess
import sys
import time
import re
import urllib.request
import urllib.error

import traceback

from mako.template import Template

import desktop_environments as desktops
import parted3.fs_module as fs
import misc.misc as misc
import pacman.pac as pac
import info
import encfs

from download import download

from installation import auto_partition
from installation import chroot
from installation import mkinitcpio
from installation import firewall

from misc.misc import InstallError

try:
    import xml.etree.cElementTree as eTree
except ImportError as err:
    import xml.etree.ElementTree as eTree

try:
    import pyalpm
except ImportError as err:
    logging.error(err)

POSTINSTALL_SCRIPT = 'postinstall.sh'
DEST_DIR = "/install"


def chroot_run(cmd):
    chroot.run(cmd, DEST_DIR)


def write_file(filecontents, filename):
    """ writes a string of data to disk """
    if not os.path.exists(os.path.dirname(filename)):
        os.makedirs(os.path.dirname(filename))

    with open(filename, "w") as fh:
        fh.write(filecontents)


class InstallationProcess(multiprocessing.Process):
    """ Installation process thread class """

    def __init__(self, settings, callback_queue, mount_devices,
                 fs_devices, alternate_package_list="", ssd=None, blvm=False):
        """ Initialize installation class """
        multiprocessing.Process.__init__(self)

        self.alternate_package_list = alternate_package_list

        self.callback_queue = callback_queue
        self.settings = settings
        self.method = self.settings.get('partition_mode')
        msg = _("Installing using the '{0}' method").format(self.method)
        self.queue_event('info', msg)

        # Check desktop selected to load packages needed
        self.desktop = self.settings.get('desktop')

        # This flag tells us if there is a lvm partition (from advanced install)
        # If it's true we'll have to add the 'lvm2' hook to mkinitcpio
        self.blvm = blvm

        if ssd is not None:
            self.ssd = ssd
        else:
            self.ssd = {}

        self.mount_devices = mount_devices

        # Set defaults
        self.desktop_manager = 'gdm'
        self.network_manager = 'NetworkManager'

        # Packages to be removed
        self.conflicts = []

        self.fs_devices = fs_devices

        self.running = True
        self.error = False

        # Initialize some vars that are correctly initialized elsewhere
        # (pylint complains if we don't do it here)
        self.auto_device = ""
        self.packages = []
        self.pacman = None
        self.vbox = "False"

    def queue_fatal_event(self, txt):
        """ Queues the fatal event and exits process """
        self.error = True
        self.running = False
        self.queue_event('error', txt)
        self.callback_queue.join()
        sys.exit(0)

    def queue_event(self, event_type, event_text=""):
        if self.callback_queue is not None:
            try:
                self.callback_queue.put_nowait((event_type, event_text))
            except queue.Full:
                pass
        else:
            print("{0}: {1}".format(event_type, event_text))

    def wait_for_empty_queue(self, timeout):
        if self.callback_queue is not None:
            tries = 0
            if timeout < 1:
                timeout = 1
            while tries < timeout and not self.callback_queue.empty():
                time.sleep(1)
                tries += 1

    def run(self):
        """ Calls run_installation and takes care of exceptions """

        try:
            self.run_installation()
        except subprocess.CalledProcessError as process_error:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            trace = traceback.format_exception(exc_type, exc_value, exc_traceback)
            logging.error(_("Error running command %s"), process_error.cmd)
            logging.error(_("Output: %s"), process_error.output)
            for line in trace:
                logging.error(line)
            self.queue_fatal_event(process_error.output)
        except (InstallError,
                pyalpm.error,
                KeyboardInterrupt,
                TypeError,
                AttributeError,
                OSError,
                IOError) as install_error:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            trace = traceback.format_exception(exc_type, exc_value, exc_traceback)
            logging.error(install_error)
            for line in trace:
                logging.error(line)
            self.queue_fatal_event(install_error)

    @misc.raise_privileges
    def run_installation(self):
        """ Run installation """

        '''
        From this point, on a warning situation, Cnchi should try to continue, so we need to catch the exception here.
        If we don't catch the exception here, it will be catched in run() and managed as a fatal error.
        On the other hand, if we want to clarify the exception message we can catch it here
        and then raise an InstallError exception.
        '''

        # Common vars
        self.packages = []

        if not os.path.exists(DEST_DIR):
            with misc.raised_privileges():
                os.makedirs(DEST_DIR)
        else:
            # If we're recovering from a failed/stoped install, there'll be
            # some mounted directories. Try to unmount them first.
            # We use unmount_all from auto_partition to do this.
            auto_partition.unmount_all(DEST_DIR)

        # Create and format partitions

        if self.method == 'automatic':
            self.auto_device = self.settings.get('auto_device')

            logging.debug(_("Creating partitions and their filesystems in %s"), self.auto_device)

            # If no key password is given a key file is generated and stored in /boot
            # (see auto_partition.py)

            auto = auto_partition.AutoPartition(dest_dir=DEST_DIR,
                                                auto_device=self.auto_device,
                                                use_luks=self.settings.get("use_luks"),
                                                luks_password=self.settings.get("luks_root_password"),
                                                use_lvm=self.settings.get("use_lvm"),
                                                use_home=self.settings.get("use_home"),
                                                bootloader=self.settings.get("bootloader"),
                                                callback_queue=self.callback_queue)
            auto.run()

            # Get mount_devices and fs_devices
            # (mount_devices will be used when configuring GRUB in modify_grub_default)
            # (fs_devices  will be used when configuring the fstab file)
            self.mount_devices = auto.get_mount_devices()
            self.fs_devices = auto.get_fs_devices()

        # Create the directory where we will mount our new root partition
        if not os.path.exists(DEST_DIR):
            os.mkdir(DEST_DIR)

        if self.method == 'alongside' or self.method == 'advanced':
            root_partition = self.mount_devices["/"]

            # NOTE: Advanced method formats root by default in installation_advanced

            # if root_partition in self.fs_devices:
            #     root_fs = self.fs_devices[root_partition]
            # else:
            #    root_fs = "ext4"

            if "/boot" in self.mount_devices:
                boot_partition = self.mount_devices["/boot"]
            else:
                boot_partition = ""

            if "swap" in self.mount_devices:
                swap_partition = self.mount_devices["swap"]
            else:
                swap_partition = ""

            # Mount root and boot partitions (only if it's needed)
            # Not doing this in automatic mode as AutoPartition class mounts the root and boot devices itself.
            txt = _("Mounting partition {0} into {1} directory").format(root_partition, DEST_DIR)
            logging.debug(txt)
            subprocess.check_call(['mount', root_partition, DEST_DIR])
            # We also mount the boot partition if it's needed
            boot_path = os.path.join(DEST_DIR, "boot")
            if not os.path.exists(boot_path):
                os.makedirs(boot_path)
            if "/boot" in self.mount_devices:
                txt = _("Mounting partition {0} into {1}/boot directory")
                txt = txt.format(boot_partition, boot_path)
                logging.debug(txt)
                subprocess.check_call(['mount', boot_partition, boot_path])

            # In advanced mode, mount all partitions (root and boot are already mounted)
            if self.method == 'advanced':
                for path in self.mount_devices:
                    if path == "":
                        # Ignore devices without a mount path (or they will be mounted at "DEST_DIR")
                        continue

                    mount_part = self.mount_devices[path]

                    if mount_part != root_partition and mount_part != boot_partition and mount_part != swap_partition:
                        if path[0] == '/':
                            path = path[1:]
                        mount_dir = os.path.join(DEST_DIR, path)
                        try:
                            if not os.path.exists(mount_dir):
                                os.makedirs(mount_dir)
                            txt = _("Mounting partition {0} into {1} directory").format(mount_part, mount_dir)
                            logging.debug(txt)
                            subprocess.check_call(['mount', mount_part, mount_dir])
                        except subprocess.CalledProcessError as process_error:
                            # We will continue as root and boot are already mounted
                            logging.warning(_("Can't mount %s in %s"), mount_part, mount_dir)
                            logging.warning(_("Command %s has failed."), process_error.cmd)
                            logging.warning(_("Output : %s"), process_error.output)
                    elif mount_part == swap_partition:
                        try:
                            logging.debug(_("Activating swap in %s"), mount_part)
                            subprocess.check_call(['swapon', swap_partition])
                        except subprocess.CalledProcessError as process_error:
                            # We can continue even if no swap is on
                            logging.warning(_("Can't activate swap in %s"), mount_part)
                            logging.warning(_("Command %s has failed."), process_error.cmd)
                            logging.warning(_("Output : %s"), process_error.output)

        # Nasty workaround:
        # If pacman was stoped and /var is in another partition than root
        # (so as to be able to resume install), database lock file will still be in place.
        # We must delete it or this new installation will fail
        db_lock = os.path.join(DEST_DIR, "var/lib/pacman/db.lck")
        if os.path.exists(db_lock):
            with misc.raised_privileges():
                os.remove(db_lock)
            logging.debug(_("%s deleted"), db_lock)

        # Create some needed folders
        folders = [
            os.path.join(DEST_DIR, 'var/lib/pacman'),
            os.path.join(DEST_DIR, 'etc/pacman.d/gnupg'),
            os.path.join(DEST_DIR, 'var/log')]

        for folder in folders:
            if not os.path.exists(folder):
                os.makedirs(folder)

        # If kernel images exists in /boot they are most likely from a failed install attempt and need
        # to be removed otherwise pyalpm will raise a fatal exception later on.
        kernel_imgs = (
            "/install/boot/vmlinuz-linux",
            "/install/boot/vmlinuz-linux-lts",
            "/install/boot/initramfs-linux.img",
            "/install/boot/initramfs-linux-fallback.img",
            "/install/boot/initramfs-linux-lts.img",
            "/install/boot/initramfs-linux-lts-fallback.img")

        for img in kernel_imgs:
            if os.path.exists(img):
                os.remove(img)

        logging.debug(_("Prepare pacman..."))
        self.prepare_pacman()
        logging.debug(_("Pacman ready"))

        logging.debug(_("Selecting packages..."))
        self.select_packages()
        logging.debug(_("Packages selected"))

        # Fix bug #263 (v86d moved from [extra] to AUR)
        if "v86d" in self.packages:
            self.packages.remove("v86d")
            logging.debug(_("Removed 'v86d' package from list"))

        logging.debug(_("Downloading packages..."))
        self.download_packages()

        logging.debug(_("Installing packages..."))
        self.install_packages()

        logging.debug(_("Configuring system..."))
        self.configure_system()

        all_ok = True

        self.running = False

        if all_ok is False:
            self.error = True
            return False
        else:
            # Installation finished successfully
            self.queue_event('finished', _("Installation finished"))
            self.error = False
            return True

    @staticmethod
    def copy_log():
        # Copy Cnchi log to new installation
        datetime = "{0}-{1}".format(time.strftime("%Y%m%d"), time.strftime("%H%M%S"))
        dst = os.path.join(DEST_DIR, "var/log/cnchi-{0}.log".format(datetime))
        pidst = os.path.join(DEST_DIR, "var/log/postinstall-{0}.log".format(datetime))
        try:
            shutil.copy("/tmp/cnchi.log", dst)
            shutil.copy("/tmp/postinstall.log", pidst)
        except FileNotFoundError:
            logging.warning(_("Can't copy Cnchi log to %s"), dst)
        except FileExistsError:
            pass

    def download_packages(self):
        """ Downloads necessary packages """
        pacman_conf_file = "/tmp/pacman.conf"
        pacman_cache_dir = os.path.join(DEST_DIR, "var/cache/pacman/pkg")

        if self.settings.get("cache") != '':
            cache_dir = self.settings.get("cache")
        else:
            cache_dir = '/var/cache/pacman/pkg'

        if self.settings.get("download_library"):
            download_library = self.settings.get("download_library")
        else:
            download_library = 'urllib'

        download.DownloadPackages(
            self.packages,
            download_library,
            pacman_conf_file,
            pacman_cache_dir,
            cache_dir,
            self.callback_queue,
            self.settings)

    def create_pacman_conf_file(self):
        """ Creates a temporary pacman.conf """
        myarch = os.uname()[-1]
        msg = _("Creating a temporary pacman.conf for {0} architecture").format(myarch)
        logging.debug(msg)

        # Template functionality. Needs Mako (see http://www.makotemplates.org/)
        template_file_name = os.path.join(self.settings.get('data'), 'pacman.tmpl')
        file_template = Template(filename=template_file_name)
        file_rendered = file_template.render(destDir=DEST_DIR, arch=myarch, desktop=self.desktop)
        write_file(file_rendered, os.path.join("/tmp", "pacman.conf"))

    def prepare_pacman(self):
        """ Configures pacman and syncs db on destination system """

        self.create_pacman_conf_file()

        dirs = ["var/cache/pacman/pkg", "var/lib/pacman"]

        for pacman_dir in dirs:
            mydir = os.path.join(DEST_DIR, pacman_dir)
            if not os.path.exists(mydir):
                os.makedirs(mydir)

        self.prepare_pacman_keyring()

        # Init pyalpm
        try:
            self.pacman = pac.Pac("/tmp/pacman.conf", self.callback_queue)
        except Exception:
            self.pacman = None
            raise InstallError(_("Can't initialize pyalpm."))

        # If we failed to download any packages earlier, try refreshing our mirrorlist
        # before ALPM attempts to download them.
        if self.settings.get('failed_download'):
            from rank_mirrors import AutoRankmirrorsThread
            retry = AutoRankmirrorsThread()
            retry.start()
            retry.join()

        # Refresh pacman databases
        result = self.pacman.refresh()

        if not result:
            txt = _("Can't refresh pacman databases.")
            logging.error(txt)
            raise InstallError(txt)

    @staticmethod
    def prepare_pacman_keyring():
        """ Add gnupg pacman files to installed system """

        # Be sure that haveged is running (liveCD)
        # haveged is a daemon that generates system entropy; this speeds up
        # critical operations in cryptographic programs such as gnupg
        # (including the generation of new keyrings)
        try:
            cmd = ["systemctl", "start", "haveged"]
            subprocess.check_call(cmd)
        except subprocess.CalledProcessError as process_error:
            logging.warning(_("Can't start haveged service"))
            logging.warning(_("Command %s failed"), process_error.cmd)
            logging.warning(_("Output: %s"), process_error.output)

        # Delete old gnupg files
        dest_path = os.path.join(DEST_DIR, "etc/pacman.d/gnupg")
        try:
            cmd = ["rm", "-rf", dest_path]
            subprocess.check_call(cmd)
            os.mkdir(dest_path)
        except subprocess.CalledProcessError as process_error:
            logging.warning(_("Error deleting old gnupg files"))
            logging.warning(_("Command %s failed"), process_error.cmd)
            logging.warning(_("Output: %s"), process_error.output)

        # Tell pacman-key to regenerate gnupg files
        try:
            cmd = ["pacman-key", "--init", "--gpgdir", dest_path]
            subprocess.check_call(cmd)
            cmd = ["pacman-key", "--populate", "--gpgdir", dest_path, "archlinux"]
            cmd = ["pacman-key", "--populate", "archlinux"]
            subprocess.check_call(cmd)
        except subprocess.CalledProcessError as process_error:
            logging.warning(_("Error regenerating gnupg files with pacman-key"))
            logging.warning(_("Command %s failed"), process_error.cmd)
            logging.warning(_("Output: %s"), process_error.output)

    def select_packages(self):
        """ Get package list from the Internet """
        self.packages = []

        if len(self.alternate_package_list) > 0:
            packages_xml = self.alternate_package_list
        else:
            # The list of packages is retrieved from an online XML to let us
            # control the pkgname in case of any modification

            self.queue_event('info', _("Getting package list..."))

            try:
                url = 'http://install.apricity.com/packages-{0}.xml'.format(info.CNCHI_VERSION[:3])
                logging.debug(_("Getting url {0}...").format(url))
                req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
                packages_xml = urllib.request.urlopen(req, timeout=10)
            except urllib.error.URLError as url_error:
                # If the installer can't retrieve the remote file Cnchi will use
                # a local copy, which might be updated or not.
                logging.warning(url_error)
                logging.debug(_("Can't retrieve remote package list, using the local file instead."))
                data_dir = self.settings.get("data")
                packages_xml = os.path.join(data_dir, 'packages.xml')
                logging.debug(_("Loading {0}").format(packages_xml))

        xml_tree = eTree.parse(packages_xml)
        xml_root = xml_tree.getroot()

        lib = desktops.LIBS

        for editions in xml_root.iter('editions'):
            for edition in editions.iter('edition'):
                name = edition.attrib.get("name").lower()
                # Add common packages to all desktops (including base)
                if name == "common":
                    for pkg in edition.iter('pkgname'):
                        self.packages.append(pkg.text)
                # Add common graphical packages
                if name == "graphic" and self.desktop != "base":
                    for pkg in edition.iter('pkgname'):
                        # If package is Desktop Manager, save the name to activate the correct service later
                        if pkg.attrib.get('dm'):
                            self.desktop_manager = pkg.attrib.get('name')
                        plib = pkg.attrib.get('lib')
                        if plib is None or (plib is not None and self.desktop in lib[plib]):
                            self.packages.append(pkg.text)
                # Add specific desktop packages
                if name == self.desktop:
                    logging.debug(_("Adding '%s' desktop packages"), self.desktop)
                    for pkg in edition.iter('pkgname'):
                        # If package is Network Manager, save the name to activate the correct service later
                        if pkg.attrib.get('nm'):
                            self.network_manager = pkg.attrib.get('name')
                        if pkg.attrib.get('conflicts'):
                            self.conflicts.append(pkg.attrib.get('conflicts'))
                        self.packages.append(pkg.text)

        # Set KDE language pack
        if self.desktop == 'kde4' or self.desktop == 'plasma5':
            pkg = ""
            base_name = 'kde-l10n-'
            lang_name = self.settings.get("language_name").lower()
            if lang_name == "english":
                # There're some English variants available but not all of them.
                lang_packs = ['en_gb']
                locale = self.settings.get('locale').split('.')[0]
                if locale in lang_packs:
                    pkg = base_name + locale
            else:
                # All the other language packs use their language code
                lang_code = self.settings.get('language_code')
                pkg = base_name + lang_code
            if len(pkg) > 0:
                logging.debug(_("Selected kde language pack: %s"), pkg)
                self.packages.append(pkg)

        # Add ntp package if user selected it in timezone screen
        if self.settings.get('use_ntp'):
            for child in xml_root.iter('ntp'):
                for pkg in child.iter('pkgname'):
                    self.packages.append(pkg.text)

        # Get packages needed for detected hardware
        try:
            import hardware.hardware as hardware

            hardware_install = hardware.HardwareInstall()
            driver_names = hardware_install.get_found_driver_names()
            if len(driver_names) > 0:
                logging.debug(_("Hardware module detected this drivers: %s"), driver_names)
            hardware_pkgs = hardware_install.get_packages()
            if len(hardware_pkgs) > 0:
                txt = " ".join(hardware_pkgs)
                logging.debug(_("Hardware module added these packages : %s"), txt)
                if 'virtualbox-guest-utils' in hardware_pkgs:
                    self.vbox = "True"
                self.packages.extend(hardware_pkgs)
        except ImportError:
            logging.warning(_("Can't import hardware module."))
        except Exception as general_error:
            logging.warning(_("Unknown error in hardware module. Output: %s"), general_error)

        # Add filesystem packages
        logging.debug(_("Adding filesystem packages"))

        cmd = ["blkid", "-c", "/dev/null", "-o", "value", "-s", "TYPE"]
        fs_types = subprocess.check_output(cmd).decode()

        fs_lib = (
            'btrfs', 'ext', 'ext2', 'ext3', 'ext4', 'fat', 'fat16', 'fat32', 'vfat',
            'f2fs', 'jfs', 'nfs', 'nilfs2', 'ntfs', 'reiserfs', 'xfs')

        for fs_index in self.fs_devices:
            fs_types = fs_types + self.fs_devices[fs_index]

        for fsys in fs_lib:
            if fsys in fs_types:
                if fsys == 'ext2' or fsys == 'ext3' or fsys == 'ext4':
                    fsys = 'ext'
                if fsys == 'fat16' or fsys == 'fat32':
                    fsys = 'vfat'
                for child in xml_root.iter(fsys):
                    for pkg in child.iter('pkgname'):
                        self.packages.append(pkg.text)

        # Check for user desired features and add them to our installation
        logging.debug(_("Check for user desired features and add them to our installation"))
        self.add_features_packages(xml_root)
        logging.debug(_("All features needed packages have been added"))

        # Add chinese fonts
        lang_code = self.settings.get("language_code")
        if lang_code == "zh_TW" or lang_code == "zh_CN":
            logging.debug(_("Selecting chinese fonts."))
            for child in xml_root.iter('chinese'):
                for pkg in child.iter('pkgname'):
                    self.packages.append(pkg.text)

        # Add bootloader packages if needed
        if self.settings.get('bootloader_install'):
            boot_loader = self.settings.get('bootloader')
            # Search boot_loader in packages.xml
            bootloader_found = False
            for child in xml_root.iter('bootloader'):
                if child.attrib.get('name') == boot_loader:
                    txt = _("Adding '%s' bootloader packages")
                    logging.debug(txt, boot_loader)
                    bootloader_found = True
                    for pkg in child.iter('pkgname'):
                        self.packages.append(pkg.text)
            if not bootloader_found:
                txt = _("Couldn't find %s bootloader packages!")
                logging.warning(txt, boot_loader)

        # Check the list of packages for empty strings and remove any that we find.
        self.packages = [pkg for pkg in self.packages if pkg != '']
        logging.debug(self.packages)

    def add_features_packages(self, xml_root):
        """ Selects packages based on user selected features """
        desktop = self.settings.get("desktop")
        lib = desktops.LIBS

        # Add necessary packages for user desired features to our install list
        for xml_features in xml_root.iter('features'):
            for xml_feature in xml_features.iter('feature'):
                feature = xml_feature.attrib.get("name")
                if self.settings.get("feature_" + feature):
                    logging.debug(_("Adding packages for '%s' feature."), feature)
                    for pkg in xml_feature.iter('pkgname'):
                        # If it's a specific gtk or qt package we have to check it
                        # against our chosen desktop.
                        plib = pkg.attrib.get('lib')
                        qt5 = pkg.attrib.get('qt5')
                        if plib is None or (plib is not None and desktop in lib[plib]):
                            if self.desktop == "plasma5" and pkg.text == "bluedevil4":
                                continue
                            elif self.desktop != "plasma5" and qt5 == "True":
                                continue
                            else:
                                logging.debug(_("Selecting package %s for feature %s"), pkg.text, feature)
                                self.packages.append(pkg.text)
                        if pkg.attrib.get('conflicts'):
                            self.conflicts.append(pkg.attrib.get('conflicts'))

        # Add libreoffice language package
        if self.settings.get('feature_office'):
            logging.debug(_("Add libreoffice language package"))
            pkg = ""
            lang_name = self.settings.get("language_name").lower()
            if lang_name == "english":
                # There're some English variants available but not all of them.
                lang_packs = ['en-GB', 'en-ZA']
                locale = self.settings.get('locale').split('.')[0]
                locale = locale.replace('_', '-')
                if locale in lang_packs:
                    pkg = "libreoffice-fresh-{0}".format(locale)
            else:
                # All the other language packs use their language code
                lang_code = self.settings.get('language_code')
                lang_code = lang_code.replace('_', '-')
                pkg = "libreoffice-fresh-{0}".format(lang_code)
            if pkg != "":
                self.packages.append(pkg)

    def install_packages(self):
        """ Start pacman installation of packages """
        logging.debug(_("Installing packages..."))

        pacman_options = {}

        result = self.pacman.install(
            pkgs=self.packages,
            conflicts=self.conflicts,
            options=pacman_options)

        if not result:
            txt = _("Can't install necessary packages. Cnchi can't continue.")
            raise InstallError(txt)

        # All downloading and installing has been done, so we hide progress bar
        self.queue_event('progress_bar', 'hide')

    def is_running(self):
        """ Checks if thread is running """
        return self.running

    def is_ok(self):
        """ Checks if an error has been issued """
        return not self.error

    @staticmethod
    def copy_network_config():
        """ Copies Network Manager configuration """
        source_nm = "/etc/NetworkManager/system-connections/"
        target_nm = os.path.join(DEST_DIR, "etc/NetworkManager/system-connections")

        # Sanity checks.  We don't want to do anything if a network
        # configuration already exists on the target
        if os.path.exists(source_nm) and os.path.exists(target_nm):
            for network in os.listdir(source_nm):
                # Skip LTSP live
                if network == "LTSP":
                    continue

                source_network = os.path.join(source_nm, network)
                target_network = os.path.join(target_nm, network)

                if os.path.exists(target_network):
                    continue

                try:
                    shutil.copy(source_network, target_network)
                except FileNotFoundError:
                    logging.warning(_("Can't copy network configuration files"))
                except FileExistsError:
                    pass

    def auto_fstab(self):
        """ Create /etc/fstab file """

        all_lines = ["# /etc/fstab: static file system information.",
                     "#",
                     "# Use 'blkid' to print the universally unique identifier for a",
                     "# device; this may be used with UUID= as a more robust way to name devices",
                     "# that works even if disks are added and removed. See fstab(5).",
                     "#",
                     "# <file system> <mount point>   <type>  <options>       <dump>  <pass>",
                     "#"]

        use_luks = self.settings.get("use_luks")
        use_lvm = self.settings.get("use_lvm")

        for mount_point in self.mount_devices:
            partition_path = self.mount_devices[mount_point]
            part_info = fs.get_info(partition_path)
            uuid = part_info['UUID']

            if partition_path in self.fs_devices:
                myfmt = self.fs_devices[partition_path]
            else:
                # It hasn't any filesystem defined, skip it.
                continue

            # Take care of swap partitions
            if "swap" in myfmt:
                # If using a TRIM supported SSD, discard is a valid mount option for swap
                if partition_path in self.ssd:
                    opts = "defaults,discard"
                else:
                    opts = "defaults"
                txt = "UUID={0} swap swap {1} 0 0".format(uuid, opts)
                all_lines.append(txt)
                logging.debug(_("Added to fstab : %s"), txt)
                continue

            crypttab_path = os.path.join(DEST_DIR, 'etc/crypttab')

            # Fix for home + luks, no lvm (from Automatic Install)
            if "/home" in mount_point and self.method == "automatic" and use_luks and not use_lvm:
                # Modify the crypttab file
                luks_root_password = self.settings.get("luks_root_password")
                if luks_root_password and len(luks_root_password) > 0:
                    # Use password and not a keyfile
                    home_keyfile = "none"
                else:
                    # Use a keyfile
                    home_keyfile = "/etc/luks-keys/home"

                os.chmod(crypttab_path, 0o666)
                with open(crypttab_path, 'a') as crypttab_file:
                    line = "cryptApricityHome /dev/disk/by-uuid/{0} {1} luks\n".format(uuid, home_keyfile)
                    crypttab_file.write(line)
                    logging.debug(_("Added to crypttab : %s"), line)
                os.chmod(crypttab_path, 0o600)

                # Add line to fstab
                txt = "/dev/mapper/cryptApricityHome {0} {1} defaults 0 0".format(mount_point, myfmt)
                all_lines.append(txt)
                logging.debug(_("Added to fstab : %s"), txt)
                continue

            # Add all LUKS partitions from Advanced Install (except root).
            if self.method == "advanced" and mount_point is not "/" and use_luks and "/dev/mapper" in partition_path:
                os.chmod(crypttab_path, 0o666)
                vol_name = partition_path[len("/dev/mapper/"):]
                with open(crypttab_path, 'a') as crypttab_file:
                    line = "{0} /dev/disk/by-uuid/{1} none luks\n".format(vol_name, uuid)
                    crypttab_file.write(line)
                    logging.debug(_("Added to crypttab : %s"), line)
                os.chmod(crypttab_path, 0o600)

                txt = "{0} {1} {2} defaults 0 0".format(partition_path, mount_point, myfmt)
                all_lines.append(txt)
                logging.debug(_("Added to fstab : %s"), txt)
                continue

            # fstab uses vfat to mount fat16 and fat32 partitions
            if "fat" in myfmt:
                myfmt = 'vfat'

            if "btrfs" in myfmt:
                self.settings.set('btrfs', True)

            # Avoid adding a partition to fstab when it has no mount point (swap has been checked above)
            if mount_point == "":
                continue

            # Create mount point on destination system if it yet doesn't exist
            full_path = os.path.join(DEST_DIR, mount_point)
            if not os.path.exists(full_path):
                os.makedirs(full_path)

            # Is ssd ?
            # Device list example: {'/dev/sdb': False, '/dev/sda': True}

            '''
            is_ssd = False
            for ssd_device in self.ssd:
                if ssd_device in partition_path:
                    is_ssd = True
            '''

            logging.debug("Device list : {0}".format(self.ssd))
            device = re.sub("[0-9]+$", "", partition_path)
            is_ssd = self.ssd.get(device)
            logging.debug("Device: {0}, SSD: {1}".format(device, is_ssd))

            # Add mount options parameters
            if not is_ssd:
                if "btrfs" in myfmt:
                    opts = 'defaults,rw,relatime,space_cache,autodefrag,inode_cache'
                elif "f2fs" in myfmt:
                    opts = 'defaults,rw,noatime'
                elif "ext3" in myfmt or "ext4" in myfmt:
                    opts = 'defaults,rw,relatime,data=ordered'
                else:
                    opts = "defaults,rw,relatime"
            else:
                # As of linux kernel version 3.7, the following
                # filesystems support TRIM: ext4, btrfs, JFS, and XFS.
                if myfmt == 'ext4' or myfmt == 'jfs' or myfmt == 'xfs':
                    opts = 'defaults,rw,noatime,discard'
                elif myfmt == 'btrfs':
                    opts = 'defaults,rw,noatime,compress=lzo,ssd,discard,space_cache,autodefrag,inode_cache'
                else:
                    opts = 'defaults,rw,noatime'

            no_check = ["btrfs", "f2fs"]

            if mount_point == "/" and myfmt not in no_check:
                chk = '1'
            else:
                chk = '0'

            if mount_point == "/":
                self.settings.set('ruuid', uuid)

            txt = "UUID={0} {1} {2} {3} 0 {4}".format(uuid, mount_point, myfmt, opts, chk)
            all_lines.append(txt)
            logging.debug(_("Added to fstab : %s"), txt)

        # Create tmpfs line in fstab
        tmpfs = "tmpfs /tmp tmpfs defaults,noatime,mode=1777 0 0"
        all_lines.append(tmpfs)
        logging.debug(_("Added to fstab : %s"), tmpfs)

        full_text = '\n'.join(all_lines) + '\n'

        fstab_path = os.path.join(DEST_DIR, 'etc/fstab')
        with open(fstab_path, 'w') as fstab_file:
            fstab_file.write(full_text)

        logging.debug(_("fstab written."))

    def set_scheduler(self):
        rule_src = os.path.join(self.settings.get('cnchi'), 'scripts/60-schedulers.rules')
        rule_dst = os.path.join(DEST_DIR, "etc/udev/rules.d/60-schedulers.rules")
        try:
            shutil.copy2(rule_src, rule_dst)
            os.chmod(rule_dst, 0o755)
        except FileNotFoundError:
            logging.debug(_("Could not copy udev rule for SSDs"))
        except FileExistsError:
            pass

    @staticmethod
    def enable_services(services):
        """ Enables all services that are in the list 'services' """
        chroot_run(["systemctl", "mask", "systemd-rfkill@.service"])
        chroot_run(["systemctl", "enable", "org.cups.cupsd.service"])
        chroot_run(["pacman-key", "--populate", "archlinux"])
        for name in services:
            path = os.path.join(DEST_DIR, "usr/lib/systemd/system/{0}.service".format(name))
            if os.path.exists(path):
                chroot_run(['systemctl', '-f', 'enable', name])
                logging.debug(_("Enabled %s service."), name)
            else:
                logging.warning(_("Can't find service %s"), name)

    @staticmethod
    def change_user_password(user, new_password):
        """ Changes the user's password """
        try:
            shadow_password = crypt.crypt(new_password, "$6${0}$".format(user))
        except:
            logging.warning(_("Error creating password hash for user %s"), user)
            return False

        try:
            chroot_run(['usermod', '-p', shadow_password, user])
        except:
            logging.warning(_("Error changing password for user %s"), user)
            return False

        return True

    @staticmethod
    def auto_timesetting():
        """ Set hardware clock """
        subprocess.check_call(["hwclock", "--systohc", "--utc"])
        shutil.copy2("/etc/adjtime", os.path.join(DEST_DIR, "etc/"))

    @staticmethod
    def update_pacman_conf():
        """ Add Apricity repo """
        path = os.path.join(DEST_DIR, "etc/pacman.conf")
        if os.path.exists(path):
            with open(path, "a") as pacman_conf:
                pacman_conf.write("\n\n")
                pacman_conf.write("[apricity-core]\n")
                pacman_conf.write("SigLevel = Never\n")
                pacman_conf.write("Server = http://apricityos.com/apricity-core/\n")
        else:
            logging.warning(_("Can't find pacman configuration file"))

    @staticmethod
    def uncomment_locale_gen(locale):
        """ Uncomment selected locale in /etc/locale.gen """

        path = os.path.join(DEST_DIR, "etc/locale.gen")

        if os.path.exists(path):
            with open(path) as gen:
                text = gen.readlines()

            with open(path, "w") as gen:
                for line in text:
                    if locale in line and line[0] == "#":
                        # remove trailing '#'
                        line = line[1:]
                    gen.write(line)
        else:
            logging.warning(_("Can't find locale.gen file"))

    @staticmethod
    def check_output(command):
        """ Helper function to run a command """
        return subprocess.check_output(command.split()).decode().strip("\n")

    def copy_cached_packages(self, cache_dir):
        """ Copy all packages from specified directory to install's target """
        # Check in case user has given a wrong folder
        if not os.path.exists(cache_dir):
            return
        self.queue_event('info', _('Copying xz files from cache...'))
        dest_dir = os.path.join(DEST_DIR, "var/cache/pacman/pkg")
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)
        self.copy_files_progress(cache_dir, dest_dir)

    def copy_files_progress(self, src, dst):
        """ Copy files updating the slides' progress bar """
        percent = 0.0
        items = os.listdir(src)
        if len(items) > 0:
            step = 1.0 / len(items)
            for item in items:
                self.queue_event('percent', percent)
                source = os.path.join(src, item)
                destination = os.path.join(dst, item)
                try:
                    shutil.copy2(source, destination)
                except (FileExistsError, shutil.Error) as file_error:
                    logging.warning(file_error)
                percent += step

    def setup_features(self):
        """ Do all set up needed by the user's selected features """
        # if self.settings.get("feature_aur"):
        #    logging.debug(_("Configuring AUR..."))

        services = ['bluetooth''org.cups.cupsd''avahi-daemon''smbd''nmbd''tlp''tlp-sleep''ntpd']

        if self.settings.get("feature_bluetooth"):
            services.append('bluetooth')

        if self.settings.get("feature_cups"):
            services.append('org.cups.cupsd')
            services.append('avahi-daemon')

        logging.debug(_("Configuring firewall..."))
        # Set firewall rules
        firewall.run(["default", "deny"])
        toallow = misc.get_network()
        if toallow:
            firewall.run(["allow", "from", toallow])
        firewall.run(["allow", "Transmission"])
        firewall.run(["allow", "SSH"])
        firewall.run(["enable"])
        services.append('ufw')

        if self.settings.get("feature_lts"):
            # FIXME: Apricity doesn't boot if linux lts is selected
            # Is something wrong with the 10_apricity file ?
            chroot_run(["chmod", "a-x", "/etc/grub.d/10_apricity"])
            chroot_run(["chmod", "a+x", "/etc/grub.d/10_linux"])
            chroot_run(["grub-mkconfig", "-o", "/boot/grub/grub.cfg"])
            chroot_run(["chmod", "a-x", "/etc/grub.d/10_linux"])
            chroot_run(["chmod", "a+x", "/etc/grub.d/10_apricity"])

        self.enable_services(services)

    def setup_display_manager(self):
        """ Configures gdm desktop manager, including autologin. """
        txt = _("Configuring GDM desktop manager...")
        self.queue_event('info', txt)
        if self.desktop in desktops.SESSIONS:
            session = desktops.SESSIONS[self.desktop]
        else:
            session = "default"

        username = self.settings.get('username')
        autologin = not self.settings.get('require_password')
        lightdm_conf_path = os.path.join(DEST_DIR, "etc/lightdm/lightdm.conf")

        try:
            # Setup LightDM as Desktop Manager

            with open(lightdm_conf_path) as lightdm_conf:
                text = lightdm_conf.readlines()

            with open(lightdm_conf_path, "w") as lightdm_conf:
                for line in text:
                    if autologin:
                        # Enable automatic login
                        if '#autologin-user=' in line:
                            line = 'autologin-user={0}\n'.format(username)
                        if '#autologin-user-timeout=0' in line:
                            line = 'autologin-user-timeout=0\n'
                    # Set correct DE session
                    if '#user-session=default' in line:
                        line = 'user-session={0}\n'.format(session)
                    lightdm_conf.write(line)

            txt = _("GDM display manager configuration completed.")
            logging.debug(txt)
        except FileNotFoundError:
            txt = _("Error while trying to configure the GDM display manager")
            logging.warning(txt)

    @staticmethod
    def alsa_mixer_setup():
        """ Sets ALSA mixer settings """

        cmds = [
            "Master 70% unmute",
            "Front 70% unmute"
            "Side 70% unmute"
            "Surround 70% unmute",
            "Center 70% unmute",
            "LFE 70% unmute",
            "Headphone 70% unmute",
            "Speaker 70% unmute",
            "PCM 70% unmute",
            "Line 70% unmute",
            "External 70% unmute",
            "FM 50% unmute",
            "Master Mono 70% unmute",
            "Master Digital 70% unmute",
            "Analog Mix 70% unmute",
            "Aux 70% unmute",
            "Aux2 70% unmute",
            "PCM Center 70% unmute",
            "PCM Front 70% unmute",
            "PCM LFE 70% unmute",
            "PCM Side 70% unmute",
            "PCM Surround 70% unmute",
            "Playback 70% unmute",
            "PCM,1 70% unmute",
            "DAC 70% unmute",
            "DAC,0 70% unmute",
            "DAC,1 70% unmute",
            "Synth 70% unmute",
            "CD 70% unmute",
            "Wave 70% unmute",
            "Music 70% unmute",
            "AC97 70% unmute",
            "Analog Front 70% unmute",
            "VIA DXS,0 70% unmute",
            "VIA DXS,1 70% unmute",
            "VIA DXS,2 70% unmute",
            "VIA DXS,3 70% unmute",
            "Mic 70% mute",
            "IEC958 70% mute",
            "Master Playback Switch on",
            "Master Surround on",
            "SB Live Analog/Digital Output Jack off",
            "Audigy Analog/Digital Output Jack off"]

        for cmd in cmds:
            chroot_run(['sh', '-c', 'amixer -c 0 sset {0}'.format(cmd)])

        # Save settings
        chroot_run(['alsactl', '-f', '/etc/asound.state', 'store'])

    @staticmethod
    def set_fluidsynth():
        """ Sets fluidsynth configuration file """

        fluid_path = os.path.join(DEST_DIR, "etc/conf.d/fluidsynth")

        if not os.path.exists(fluid_path):
            return

        audio_system = "alsa"

        pulse_path = os.path.join(DEST_DIR, "usr/bin/pulseaudio")
        if os.path.exists(pulse_path):
            audio_system = "pulse"

        with open(fluid_path, "w") as fluid_conf:
            fluid_conf.write('# Created by Cnchi, Apricity OS installer\n')
            fluid_conf.write('SYNTHOPTS="-is -a {0} -m alsa_seq -r 48000"\n\n'.format(audio_system))

    def configure_system(self):
        """ Final install steps
            Set clock, language, timezone
            Run mkinitcpio
            Populate pacman keyring
            Setup systemd services
            ... and more """

        self.queue_event('pulse', 'start')
        self.queue_event('info', _("Configuring your new system"))

        # This mounts (binds) /dev and others to /DEST_DIR/dev and others
        chroot.mount_special_dirs(DEST_DIR)

        self.auto_fstab()
        logging.debug(_("fstab file generated."))

        # If SSD was detected copy udev rule for deadline scheduler
        if self.ssd:
            self.set_scheduler()
            logging.debug(_("SSD udev rule copied successfully"))

        # Copy configured networks in Live medium to target system
        if self.network_manager == 'NetworkManager':
            self.copy_network_config()
        elif self.network_manager == 'netctl':
            # Copy network profile when using netctl (and not networkmanager)
            # netctl is used in the 'base' desktop option
            # Nowadays nearly everybody uses dhcp. If user wants to use a fixed IP the profile must be
            # edited by himself. Maybe we could ease this process?
            profile = 'ethernet-dhcp'
            # if misc.is_wireless_enabled():
            #     # TODO: We should port wifi-menu from netctl package here.
            #     profile = 'wireless-wpa'

            # TODO: Just copying the default profile is NOT an elegant solution
            logging.debug(_("Cnchi will configure netctl using the %s profile"), profile)
            src_path = os.path.join(DEST_DIR, 'etc/netctl/examples', profile)
            dst_path = os.path.join(DEST_DIR, 'etc/netctl', profile)

            try:
                shutil.copy(src_path, dst_path)
            except FileNotFoundError:
                logging.warning(_("Can't copy network configuration profiles"))
            except FileExistsError:
                pass
            # Enable our profile
            chroot_run(['netctl', 'enable', profile])
            # logging.warning(_('Netctl is installed. Please edit %s to finish your network configuration.'), dst_path)

        logging.debug(_("Network configuration copied."))

        # Copy mirror list
        mirrorlist_path = os.path.join(DEST_DIR, 'etc/pacman.d/mirrorlist')
        try:
            shutil.copy2('/etc/pacman.d/mirrorlist', mirrorlist_path)
            logging.debug(_("Mirror list copied."))
        except FileNotFoundError:
            logging.warning(_("Can't copy mirrorlist file"))
        except FileExistsError:
            pass

        # Add Apricity repo to /etc/pacman.conf
        self.update_pacman_conf()

        logging.debug(_("Generated /etc/pacman.conf"))

        # Enable services
        services = []
        if self.desktop != "base":
            services.append(self.desktop_manager)
        services.extend(["ModemManager", self.network_manager, "remote-fs.target", "haveged"])
        self.enable_services(services)

        # Enable ntp service
        if self.settings.get("use_ntp"):
            self.enable_services(["ntpd"])

        # Set timezone
        zoneinfo_path = os.path.join("/usr/share/zoneinfo", self.settings.get("timezone_zone"))
        chroot_run(['ln', '-s', zoneinfo_path, "/etc/localtime"])

        logging.debug(_("Timezone set."))

        # Wait FOREVER until the user sets his params
        # FIXME: We can wait here forever!
        while self.settings.get('user_info_done') is False:
            # Wait five seconds and try again
            time.sleep(5)

        # Set user parameters
        username = self.settings.get('username')
        fullname = self.settings.get('fullname')
        password = self.settings.get('password')
        hostname = self.settings.get('hostname')

        sudoers_dir = os.path.join(DEST_DIR, "etc/sudoers.d")
        if not os.path.exists(sudoers_dir):
            os.mkdir(sudoers_dir, 0o710)
        sudoers_path = os.path.join(sudoers_dir, "10-installer")
        try:
            with open(sudoers_path, "w") as sudoers:
                sudoers.write('{0} ALL=(ALL) ALL\n'.format(username))
            os.chmod(sudoers_path, 0o440)
            logging.debug(_("Sudo configuration for user %s done."), username)
        except IOError as io_error:
            # Do not fail if can't write 10-installer file. Something bad must be happening, though.
            logging.error(io_error)

        # Configure detected hardware
        try:
            import hardware.hardware as hardware

            hardware_install = hardware.HardwareInstall()
            logging.debug(_("Running post-install scripts from hardware module..."))
            hardware_install.post_install(DEST_DIR)
        except ImportError:
            logging.warning(_("Can't import hardware module."))
        except Exception as general_error:
            logging.warning(_("Unknown error in hardware module. Output: %s"), general_error)

        # Setup user

        default_groups = 'lp,video,network,storage,wheel,audio'

        if self.vbox == "True":
            # Why there is no vboxusers group? Add it ourselves.
            chroot_run(['groupadd', 'vboxusers'])
            default_groups += ',vboxusers,vboxsf'
            self.enable_services(["vboxservice"])

        if self.settings.get('require_password') is False:
            # Prepare system for autologin. LightDM needs the user to be in the autologin group.
            chroot_run(['groupadd', 'autologin'])
            default_groups += ',autologin'

        cmd = ['useradd', '-m', '-s', '/bin/bash', '-g', 'users', '-G', default_groups, username]
        chroot_run(cmd)
        logging.debug(_("User %s added."), username)

        self.change_user_password(username, password)

        cmd = ['chfn', '-f', fullname, username]
        chroot_run(cmd)

        cmd = ['chown', '-R', '{0}:users'.format(username), os.path.join("/home", username)]
        chroot_run(cmd)

        hostname_path = os.path.join(DEST_DIR, "etc/hostname")
        if not os.path.exists(hostname_path):
            with open(hostname_path, "w") as hostname_file:
                hostname_file.write(hostname)

        logging.debug(_("Hostname set to %s."), hostname)

        # User password is the root password
        self.change_user_password('root', password)
        logging.debug(_("Set the same password to root."))

        # Generate locales
        keyboard_layout = self.settings.get("keyboard_layout")
        keyboard_variant = self.settings.get("keyboard_variant")
        locale = self.settings.get("locale")
        self.queue_event('info', _("Generating locales..."))

        self.uncomment_locale_gen(locale)

        chroot_run(['locale-gen'])
        locale_conf_path = os.path.join(DEST_DIR, "etc/locale.conf")
        with open(locale_conf_path, "w") as locale_conf:
            locale_conf.write('LANG={0}\n'.format(locale))
            locale_conf.write('LC_COLLATE={0}\n'.format(locale))

        environment_path = os.path.join(DEST_DIR, "etc/environment")
        with open(environment_path, "w") as environment:
            environment.write('LANG={0}\n'.format(locale))

        # Set /etc/vconsole.conf
        vconsole_conf_path = os.path.join(DEST_DIR, "etc/vconsole.conf")
        with open(vconsole_conf_path, "w") as vconsole_conf:
            vconsole_conf.write('KEYMAP={0}\n'.format(keyboard_layout))

        self.queue_event('info', _("Adjusting hardware clock..."))
        self.auto_timesetting()

        if self.desktop != "base":
            # Set /etc/X11/xorg.conf.d/10-keyboard.conf for the xkblayout
            logging.debug(_("Set /etc/X11/xorg.conf.d/10-keyboard.conf for the xkblayout"))
            xorg_conf_dir = os.path.join(DEST_DIR, "etc/X11/xorg.conf.d")
            if not os.path.exists(xorg_conf_dir):
                os.mkdir(xorg_conf_dir, 0o755)
            xorg_conf_xkb_path = os.path.join(xorg_conf_dir, "10-keyboard.conf")
            try:
                with open(xorg_conf_xkb_path, "w") as xorg_conf_xkb:
                    xorg_conf_xkb.write("# Read and parsed by systemd-localed. It's probably wise not to edit this file\n")
                    xorg_conf_xkb.write('# manually too freely.\n')
                    xorg_conf_xkb.write('Section "InputClass"\n')
                    xorg_conf_xkb.write('        Identifier "system-keyboard"\n')
                    xorg_conf_xkb.write('        MatchIsKeyboard "on"\n')
                    xorg_conf_xkb.write('        Option "XkbLayout" "{0}"\n'.format(keyboard_layout))
                    if len(keyboard_variant) > 0:
                        xorg_conf_xkb.write('        Option "XkbVariant" "{0}"\n'.format(keyboard_variant))
                    xorg_conf_xkb.write('EndSection\n')
                logging.debug(_("10-keyboard.conf written."))
            except IOError as io_error:
                # Do not fail if 10-keyboard.conf can't be created. Something bad must be happening, though.
                logging.error(io_error)

        # Install configs for root
        chroot_run(['cp', '-av', '/etc/skel/.', '/root/'])

        self.queue_event('info', _("Configuring hardware ..."))

        # Copy generated xorg.conf to target
        if os.path.exists("/etc/X11/xorg.conf"):
            shutil.copy2('/etc/X11/xorg.conf', os.path.join(DEST_DIR, 'etc/X11/xorg.conf'))

        # Configure ALSA
        self.alsa_mixer_setup()
        logging.debug(_("Updated Alsa mixer settings"))

        # Set pulse
        if os.path.exists(os.path.join(DEST_DIR, "usr/bin/pulseaudio-ctl")):
            chroot_run(['pulseaudio-ctl', 'normal'])

        # Set fluidsynth audio system (in our case, pulseaudio)
        self.set_fluidsynth()
        logging.debug(_("Updated fluidsynth configuration file"))

        # Let's start without using hwdetect for mkinitcpio.conf.
        # It should work out of the box most of the time.
        # This way we don't have to fix deprecated hooks.
        # NOTE: With LUKS or LVM maybe we'll have to fix deprecated hooks.
        self.queue_event('info', _("Configuring System Startup..."))
        mkinitcpio.run(DEST_DIR, self.settings, self.mount_devices, self.blvm)

        logging.debug(_("Call Cnchi post-install script"))
        # Call post-install script to execute (g,k)settings commands or install openbox defaults
        script_path_postinstall = os.path.join(self.settings.get('cnchi'), "scripts", POSTINSTALL_SCRIPT)
        try:
            subprocess.check_call([
                "/usr/bin/bash",
                script_path_postinstall,
                username,
                DEST_DIR,
                self.desktop,
                keyboard_layout,
                keyboard_variant,
                self.vbox],
                timeout=300)
            logging.debug(_("Post install script completed successfully."))
        except subprocess.CalledProcessError as process_error:
            # Even though Post-install script call has failed we will try to continue with the installation.
            logging.error(_("Error running post-install script"))
            logging.error(_("Command %s failed"), process_error.cmd)
            logging.error(_("Output: %s"), process_error.output)
        except subprocess.TimeoutExpired as timeout_error:
            logging.error(timeout_error)

        if self.desktop != "base":
            # Set lightdm config including autologin if selected
            self.setup_display_manager()

        # Configure user features (firewall, libreoffice language pack, ...)
        self.setup_features()

        # Encrypt user's home directory if requested
        # FIXME: This is not working atm
        if self.settings.get('encrypt_home'):
            logging.debug(_("Encrypting user home dir..."))
            encfs.setup(username, DEST_DIR)
            logging.debug(_("User home dir encrypted"))

        # Install boot loader (always after running mkinitcpio)
        if self.settings.get('bootloader_install'):
            try:
                logging.debug(_("Installing bootloader..."))
                from installation import bootloader

                boot_loader = bootloader.Bootloader(DEST_DIR, self.settings, self.mount_devices)
                boot_loader.install()
            except Exception as general_error:
                logging.warning(_("While installing boot loader Cnchi encountered this error: %s"), general_error)

        # This unmounts (unbinds) /dev and others to /DEST_DIR/dev and others
        chroot.umount_special_dirs(DEST_DIR)

        # Copy installer log to the new installation (just in case something goes wrong)
        logging.debug(_("Copying install log to /var/log."))
        self.copy_log()

        self.queue_event('pulse', 'stop')

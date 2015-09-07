#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  bootloader.py
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

""" Bootloader installation """

import logging
import os
import shutil
import subprocess
import re

import parted3.fs_module as fs

from installation import chroot

import random
import string

# When testing, no _() is available
try:
    _("")
except NameError as err:
    def _(message):
        return message


class Bootloader(object):
    def __init__(self, dest_dir, settings, mount_devices):
        self.dest_dir = dest_dir
        self.settings = settings
        self.mount_devices = mount_devices
        self.method = settings.get("partition_mode")
        self.root_device = self.mount_devices["/"]
        self.root_uuid = fs.get_info(self.root_device)['UUID']

        if "swap" in self.mount_devices:
            swap_partition = self.mount_devices["swap"]
            self.swap_uuid = fs.get_info(swap_partition)['UUID']

        if "/boot" in self.mount_devices:
            boot_device = self.mount_devices["/boot"]
            self.boot_uuid = fs.get_info(boot_device)['UUID']

    def install(self):
        """ Installs the bootloader """

        # Freeze and unfreeze xfs filesystems to enable bootloader installation on xfs filesystems
        self.freeze_unfreeze_xfs()

        bootloader = self.settings.get('bootloader').lower()
        if bootloader == "grub2":
            self.install_grub()
        elif bootloader == "gummiboot":
            logging.debug(_("Cnchi will install the Gummiboot loader"))
            self.install_gummiboot()

    def install_grub(self):
        self.modify_grub_default()
        self.prepare_grub_d()

        if os.path.exists('/sys/firmware/efi'):
            logging.debug(_("Cnchi will install the Grub2 (efi) loader"))
            self.install_grub2_efi()
        else:
            logging.debug(_("Cnchi will install the Grub2 (bios) loader"))
            self.install_grub2_bios()

        self.check_root_uuid_in_grub()

    def check_root_uuid_in_grub(self):
        """ Checks grub.cfg for correct root UUID """
        if len(self.root_uuid) == 0:
            logging.warning(_("'ruuid' variable is not set. I can't check root UUID in grub.cfg, let's hope it's ok"))
            return

        ruuid_str = 'root=UUID={0}'.format(self.root_uuid)

        cmdline_linux = self.settings.get('GRUB_CMDLINE_LINUX')
        if cmdline_linux is None:
            cmdline_linux = ""

        cmdline_linux_default = self.settings.get('GRUB_CMDLINE_LINUX_DEFAULT')
        if cmdline_linux_default is None:
            cmdline_linux_default = ""

        boot_command = 'linux /vmlinuz-linux {0} {1} {2}\n'.format(ruuid_str, cmdline_linux, cmdline_linux_default)

        pattern = re.compile("menuentry 'Apricity OS'[\s\S]*initramfs-linux.img\n}")

        cfg = os.path.join(self.dest_dir, "boot/grub/grub.cfg")
        with open(cfg) as grub_file:
            parse = grub_file.read()

        if not self.settings.get('use_luks') and ruuid_str not in parse:
            entry = pattern.search(parse)
            if entry:
                logging.debug(_("Wrong uuid in grub.cfg, Cnchi will try to fix it."))
                new_entry = re.sub("linux\t/vmlinuz.*quiet\n", boot_command, entry.group())
                parse = parse.replace(entry.group(), new_entry)

                with open(cfg, 'w') as grub_file:
                    grub_file.write(parse)

    def modify_grub_default(self):
        """ If using LUKS as root, we need to modify GRUB_CMDLINE_LINUX
        GRUB_CMDLINE_LINUX : Command-line arguments to add to menu entries for the Linux kernel.
        GRUB_CMDLINE_LINUX_DEFAULT : Unless ‘GRUB_DISABLE_RECOVERY’ is set to ‘true’, two menu
            entries will be generated for each Linux kernel: one default entry and one entry
            for recovery mode. This option lists command-line arguments to add only to the default
            menu entry, after those listed in ‘GRUB_CMDLINE_LINUX’. """

        plymouth_bin = os.path.join(self.dest_dir, "usr/bin/plymouth")
        if os.path.exists(plymouth_bin):
            use_splash = "splash"
        else:
            use_splash = ""

        if "swap" in self.mount_devices:
            cmd_linux_default = 'resume=UUID={0} quiet {1}'.format(self.swap_uuid, use_splash)
        else:
            cmd_linux_default = 'quiet {0}'.format(use_splash)

        self.set_grub_option("GRUB_THEME", "/boot/grub/themes/Elegance_Dark/theme.txt")
        self.set_grub_option("GRUB_CMDLINE_LINUX_DEFAULT", cmd_linux_default)
        self.set_grub_option("GRUB_DISTRIBUTOR", "Apricity")

        if self.settings.get('use_luks'):
            # Let GRUB automatically add the kernel parameters for root encryption
            luks_root_volume = self.settings.get('luks_root_volume')

            logging.debug("Luks Root Volume: %s", luks_root_volume)

            root_device = self.root_device

            if self.method == "advanced" and self.settings.get('use_luks_in_root'):
                # Special case, in advanced when using luks in root device, we store it in luks_root_device
                root_device = self.settings.get('luks_root_device')

            root_uuid = fs.get_info(root_device)['UUID']

            logging.debug("Root device: %s", root_device)

            cmd_linux = "cryptdevice=/dev/disk/by-uuid/{0}:{1}".format(root_uuid, luks_root_volume)

            if self.settings.get("luks_root_password") == "":
                # No luks password, so user wants to use a keyfile
                cmd_linux += " cryptkey=/dev/disk/by-uuid/{0}:ext2:/.keyfile-root".format(self.boot_uuid)

            # Store grub line in settings, we'll use it later in check_root_uuid_in_grub()
            self.settings.set('GRUB_CMDLINE_LINUX', cmd_linux)
            # Store grub line in /etc/default/grub file
            self.set_grub_option("GRUB_CMDLINE_LINUX", cmd_linux)

        logging.debug(_("/etc/default/grub configuration completed successfully."))

    def set_grub_option(self, option, cmd):
        """ Changes a grub setup option in /etc/default/grub """
        try:
            default_grub = os.path.join(self.dest_dir, "etc/default", "grub")

            with open(default_grub) as grub_file:
                lines = [x.strip() for x in grub_file.readlines()]

            option_found = False

            for i in range(len(lines)):
                if option + "=" in lines[i]:
                    option_found = True
                    lines[i] = '{0}="{1}"\n'.format(option, cmd)

            if option_found:
                # Option was found and changed, store our changes
                with open(default_grub, 'w') as grub_file:
                    grub_file.write("\n".join(lines) + "\n")
            else:
                # Option was not found. Thus, append new option
                with open(default_grub, 'a') as grub_file:
                    grub_file.write('{0}="{1}"\n'.format(option, cmd))

            logging.debug('Set %s="%s" in /etc/default/grub', option, cmd)
        except Exception as general_error:
            logging.error("Can't modify /etc/default/grub")
            logging.error(general_error)

    def prepare_grub_d(self):
        """ Copies 10_apricity script into /etc/grub.d/ """
        grub_d_dir = os.path.join(self.dest_dir, "etc/grub.d")
        script_dir = os.path.join(self.settings.get("cnchi"), "scripts")
        script = "10_apricity"

        if not os.path.exists(grub_d_dir):
            os.makedirs(grub_d_dir)

        script_path = os.path.join(script_dir, script)
        if os.path.exists(script_path):
            try:
                shutil.copy2(script_path, grub_d_dir)
                os.chmod(os.path.join(grub_d_dir, script), 0o755)
            except FileNotFoundError:
                logging.debug(_("Could not copy %s to grub.d"), script)
            except FileExistsError:
                pass
        else:
            logging.warning("Can't find script %s", script_path)

    def install_grub2_bios(self):
        """ Install Grub2 bootloader in a BIOS system """
        grub_location = self.settings.get('bootloader_device')
        txt = _("Installing GRUB(2) BIOS boot loader in {0}").format(grub_location)
        logging.info(txt)

        # /dev and others need to be mounted (binded).
        # We call mount_special_dirs here just to be sure
        chroot.mount_special_dirs(self.dest_dir)

        grub_install = ['grub-install',
                        '--directory=/usr/lib/grub/i386-pc',
                        '--target=i386-pc',
                        '--boot-directory=/boot',
                        '--recheck']

        if len(grub_location) > len("/dev/sdX"):  # Use --force when installing in /dev/sdXY
            grub_install.append("--force")

        grub_install.append(grub_location)

        try:
            chroot.run(grub_install, self.dest_dir)
        except subprocess.CalledProcessError as process_error:
            logging.error(_('Command grub-install failed. Error output: %s'), process_error.output)
        except subprocess.TimeoutExpired:
            logging.error(_('Command grub-install timed out.'))
        except Exception as general_error:
            logging.error(_('Command grub-install failed. Unknown Error: %s'), general_error)

        self.install_grub2_locales()

        self.copy_grub2_theme_files()

        # Add -l option to os-prober's umount call so that it does not hang
        self.apply_osprober_patch()

        # Run grub-mkconfig last
        logging.debug(_("Running grub-mkconfig..."))
        locale = self.settings.get("locale")
        try:
            cmd = ['sh', '-c', 'LANG={0} grub-mkconfig -o /boot/grub/grub.cfg'.format(locale)]
            chroot.run(cmd, self.dest_dir, 300)
        except subprocess.TimeoutExpired:
            msg = _("grub-mkconfig does not respond. Killing grub-mount and os-prober so we can continue.")
            logging.error(msg)
            subprocess.check_call(['killall', 'grub-mount'])
            subprocess.check_call(['killall', 'os-prober'])

        cfg = os.path.join(self.dest_dir, "boot/grub/grub.cfg")
        with open(cfg) as grub_cfg:
            if "Apricity" in grub_cfg.read():
                txt = _("GRUB(2) BIOS has been successfully installed.")
                logging.info(txt)
                self.settings.set('bootloader_installation_successful', True)
            else:
                txt = _("ERROR installing GRUB(2) BIOS.")
                logging.warning(txt)
                self.settings.set('bootloader_installation_successful', False)

    @staticmethod
    def random_generator(size=4, chars=string.ascii_lowercase + string.digits):
        """ Generates a random string to be used as an identifier for the UEFI bootloader_id """
        return ''.join(random.choice(chars) for x in range(size))

    def install_grub2_efi(self):
        """ Install Grub2 bootloader in a UEFI system """
        uefi_arch = "x86_64"
        spec_uefi_arch = "x64"
        spec_uefi_arch_caps = "X64"
        bootloader_id = 'apricity_grub' if not os.path.exists('/install/boot/efi/EFI/apricity_grub') else \
            'apricity_grub_{0}'.format(self.random_generator())

        txt = _("Installing GRUB(2) UEFI {0} boot loader").format(uefi_arch)
        logging.info(txt)

        grub_install = [
            'grub-install',
            '--target={0}-efi'.format(uefi_arch),
            '--efi-directory=/install/boot/efi',
            '--bootloader-id={0}'.format(bootloader_id),
            '--boot-directory=/install/boot',
            '--recheck']
        load_module = ['modprobe', '-a', 'efivarfs']

        try:
            subprocess.call(load_module, timeout=15)
            subprocess.check_call(grub_install, timeout=120)
        except subprocess.CalledProcessError as process_error:
            logging.error('Command grub-install failed. Error output: %s', process_error.output)
        except subprocess.TimeoutExpired:
            logging.error('Command grub-install timed out.')
        except Exception as general_error:
            logging.error('Command grub-install failed. Unknown Error: %s', general_error)

        self.install_grub2_locales()

        self.copy_grub2_theme_files()

        # Copy grub into dirs known to be used as default by some OEMs if they do not exist yet.
        grub_defaults = [os.path.join(self.dest_dir, "boot/efi/EFI/BOOT", "BOOT{0}.efi".format(spec_uefi_arch_caps)),
                         os.path.join(self.dest_dir, "boot/efi/EFI/Microsoft/Boot", 'bootmgfw.efi')]

        grub_path = os.path.join(self.dest_dir, "boot/efi/EFI/apricity_grub", "grub{0}.efi".format(spec_uefi_arch))

        for grub_default in grub_defaults:
            path = grub_default.split()[0]
            if not os.path.exists(path):
                msg = _("No OEM loader found in %s. Copying Grub(2) into dir.")
                logging.info(msg, path)
                os.makedirs(path)
                msg_failed = _("Copying Grub(2) into OEM dir failed: %s")
                try:
                    shutil.copy(grub_path, grub_default)
                except FileNotFoundError:
                    logging.warning(msg_failed, _("File not found."))
                except FileExistsError:
                    logging.warning(msg_failed, _("File already exists."))
                except Exception as general_error:
                    logging.warning(msg_failed, general_error)

        # Copy uefi shell if none exists in /boot/efi/EFI
        shell_src = "/usr/share/cnchi/grub2-theme/shellx64_v2.efi"
        shell_dst = os.path.join(self.dest_dir, "boot/efi/EFI/")
        try:
            shutil.copy2(shell_src, shell_dst)
        except FileNotFoundError:
            logging.warning(_("UEFI Shell drop-in not found at %s"), shell_src)
        except FileExistsError:
            pass
        except Exception as general_error:
            logging.warning(_("UEFI Shell drop-in could not be copied."))
            logging.warning(general_error)

        # Run grub-mkconfig last
        logging.info(_("Generating grub.cfg"))

        # /dev and others need to be mounted (binded).
        # We call mount_special_dirs here just to be sure
        chroot.mount_special_dirs(self.dest_dir)

        # Add -l option to os-prober's umount call so that it does not hang
        self.apply_osprober_patch()

        logging.debug(_("Running grub-mkconfig..."))
        locale = self.settings.get("locale")
        try:
            cmd = ['sh', '-c', 'LANG={0} grub-mkconfig -o /boot/grub/grub.cfg'.format(locale)]
            chroot.run(cmd, self.dest_dir, 300)
        except subprocess.TimeoutExpired:
            txt = _("grub-mkconfig appears to be hung. Killing grub-mount and os-prober so we can continue.")
            logging.error(txt)
            subprocess.check_call(['killall', 'grub-mount'])
            subprocess.check_call(['killall', 'os-prober'])

        paths = [os.path.join(self.dest_dir, "boot/grub/x86_64-efi/core.efi"),
                 os.path.join(self.dest_dir, "boot/efi/EFI/{0}".format(bootloader_id), "grub{0}.efi".format(spec_uefi_arch))]

        exists = True

        for path in paths:
            if not os.path.exists(path):
                exists = False
                logging.debug(_("Path '%s' doesn't exist, when it should"), path)

        if exists:
            txt = _("GRUB(2) UEFI install completed successfully")
            logging.info(txt)
            self.settings.set('bootloader_installation_successful', True)
        else:
            txt = _("GRUB(2) UEFI install may not have completed successfully.")
            logging.warning(txt)
            self.settings.set('bootloader_installation_successful', False)

    def apply_osprober_patch(self):
        """ Adds -l option to os-prober's umount call so that it does not hang """
        osp_path = os.path.join(self.dest_dir, "usr/lib/os-probes/50mounted-tests")
        if os.path.exists(osp_path):
            with open(osp_path) as osp:
                text = osp.read().replace("umount", "umount -l")
            with open(osp_path, 'w') as osp:
                osp.write(text)
            logging.debug(_("50mounted-tests file patched successfully"))
        else:
            logging.warning(_("Failed to patch 50mounted-tests, file not found."))

    def copy_grub2_theme_files(self):
        """ Copy grub2 theme files to /boot """
        logging.info(_("Copying GRUB(2) Theme Files"))
        theme_dir_src = "/usr/share/cnchi/grub2-theme/Elegance_Dark"
        theme_dir_dst = os.path.join(self.dest_dir, "boot/grub/themes/Elegance_Dark")
        try:
            shutil.copytree(theme_dir_src, theme_dir_dst)
        except FileNotFoundError:
            logging.warning(_("Grub2 theme files not found"))
        except FileExistsError:
            logging.warning(_("Grub2 theme files already exist."))

    def install_grub2_locales(self):
        """ Install Grub2 locales """
        logging.info(_("Installing Grub2 locales."))
        dest_locale_dir = os.path.join(self.dest_dir, "boot/grub/locale")

        if not os.path.exists(dest_locale_dir):
            os.makedirs(dest_locale_dir)

        grub_mo = os.path.join(self.dest_dir, "usr/share/locale/en@quot/LC_MESSAGES/grub.mo")

        try:
            shutil.copy2(grub_mo, os.path.join(dest_locale_dir, "en.mo"))
        except FileNotFoundError:
            logging.warning(_("Can't install GRUB(2) locale."))
        except FileExistsError:
            # Ignore if already exists
            pass

    def install_gummiboot(self):
        """ Install Gummiboot bootloader to the EFI System Partition """
        # Setup bootloader menu
        menu_dir = os.path.join(self.dest_dir, "boot/loader")
        os.makedirs(menu_dir)
        menu_path = os.path.join(menu_dir, "loader.conf")
        with open(menu_path, 'w') as menu_file:
            menu_file.write("default apricity")

        # Setup boot entries
        conf = {}

        if not self.settings.get('use_luks'):
            conf['default'] = []
            conf['default'].append("title\tApricity\n")
            conf['default'].append("linux\t/vmlinuz-linux\n")
            conf['default'].append("initrd\t/initramfs-linux.img\n")
            conf['default'].append("options\troot=UUID={0} rw quiet\n\n".format(self.root_uuid))

            conf['fallback'] = []
            conf['fallback'].append("title\tApricity (fallback)\n")
            conf['fallback'].append("linux\t/vmlinuz-linux\n")
            conf['fallback'].append("initrd\t/initramfs-linux-fallback.img\n")
            conf['fallback'].append("options\troot=UUID={0} rw quiet\n\n".format(self.root_uuid))

            if self.settings.get('feature_lts'):
                conf['lts'] = []
                conf['lts'].append("title\tApricity LTS\n")
                conf['lts'].append("linux\t/vmlinuz-linux-lts\n")
                conf['lts'].append("initrd\t/initramfs-linux-lts.img\n")
                conf['lts'].append("options\troot=UUID={0} rw quiet\n\n".format(self.root_uuid))

                conf['lts_fallback'] = []
                conf['lts_fallback'].append("title\tApricity LTS (fallback)\n\n")
                conf['lts_fallback'].append("linux\t/vmlinuz-linux-lts\n")
                conf['lts_fallback'].append("initrd\t/initramfs-linux-lts-fallback.img\n")
                conf['lts_fallback'].append("options\troot=UUID={0} rw quiet\n\n".format(self.root_uuid))
        else:
            luks_root_volume = self.settings.get('luks_root_volume')
            luks_root_volume_device = "/dev/mapper/{0}".format(luks_root_volume)
            luks_root_volume_uuid = fs.get_info(luks_root_volume_device)['UUID']

            # In automatic mode, root_device is in self.mount_devices, as it should be
            root_device = self.root_device

            if self.method == "advanced" and self.settings.get('use_luks_in_root'):
                root_device = self.settings.get('luks_root_device')

            root_uuid = fs.get_info(root_device)['UUID']

            key = ""
            if self.settings.get("luks_root_password") == "":
                key = "cryptkey=UUID={0}:ext2:/.keyfile-root".format(self.boot_uuid)

            root_uuid_line = "cryptdevice=UUID={0}:{1} {2} root=UUID={3} rw quiet"
            root_uuid_line = root_uuid_line.format(root_uuid, luks_root_volume, key, luks_root_volume_uuid)

            conf['default'] = []
            conf['default'].append("title\tApricity\n")
            conf['default'].append("linux\t/vmlinuz-linux\n")
            conf['default'].append("options\tinitrd=/initramfs-linux.img {0}\n\n".format(root_uuid_line))

            conf['fallback'] = []
            conf['fallback'].append("title\tApricity (fallback)\n")
            conf['fallback'].append("linux\t/vmlinuz-linux\n")
            conf['fallback'].append("options\tinitrd=/initramfs-linux-fallback.img {0}\n\n".format(root_uuid_line))

            if self.settings.get('feature_lts'):
                conf['lts'] = []
                conf['lts'].append("title\tApricity LTS\n")
                conf['lts'].append("linux\t/vmlinuz-linux-lts\n")
                conf['lts'].append("options\tinitrd=/initramfs-linux-lts.img {0}\n\n".format(root_uuid_line))

                conf['lts_fallback'] = []
                conf['lts_fallback'].append("title\tApricity LTS (fallback)\n")
                conf['lts_fallback'].append("linux\t/vmlinuz-linux-lts\n")
                conf['lts_fallback'].append("options\tinitrd=/initramfs-linux-lts-fallback.img {0}\n\n".format(root_uuid_line))

        # Write boot entries
        entries_dir = os.path.join(self.dest_dir, "boot/loader/entries")
        os.makedirs(entries_dir)

        entry_path = os.path.join(entries_dir, "apricity.conf")
        with open(entry_path, 'w') as entry_file:
            for line in conf['default']:
                entry_file.write(line)

        entry_path = os.path.join(entries_dir, "apricity-fallback.conf")
        with open(entry_path, 'w') as entry_file:
            for line in conf['fallback']:
                entry_file.write(line)

        if self.settings.get('feature_lts'):
            entry_path = os.path.join(entries_dir, "apricity-lts.conf")
            with open(entry_path, 'w') as entry_file:
                for line in conf['lts']:
                    entry_file.write(line)

            entry_path = os.path.join(entries_dir, "apricity-lts-fallback.conf")
            with open(entry_path, 'w') as entry_file:
                for line in conf['lts_fallback']:
                    entry_file.write(line)

        # Install bootloader
        logging.debug(_("Installing gummiboot bootloader..."))
        try:
            chroot.mount_special_dirs(self.dest_dir)
            cmd = ['gummiboot', '--path=/boot', 'install']
            chroot.run(cmd, self.dest_dir, 300)
            chroot.umount_special_dirs(self.dest_dir)
            logging.info(_("Gummiboot install completed successfully"))
            self.settings.set('bootloader_installation_successful', True)
        except subprocess.CalledProcessError as process_error:
            logging.error(_('Command gummiboot failed. Error output: %s'), process_error.output)
            self.settings.set('bootloader_installation_successful', False)
        except subprocess.TimeoutExpired:
            logging.error(_('Command gummiboot timed out.'))
            self.settings.set('bootloader_installation_successful', False)
        except Exception as general_error:
            logging.error(_('Command gummiboot failed. Unknown Error: %s'), general_error)
            self.settings.set('bootloader_installation_successful', False)

    def freeze_unfreeze_xfs(self):
        """ Freeze and unfreeze xfs, as hack for grub(2) installing """
        if not os.path.exists("/usr/bin/xfs_freeze"):
            return

        xfs_boot = False
        xfs_root = False

        try:
            subprocess.check_call(["sync"])
            with open("/proc/mounts") as mounts_file:
                mounts = mounts_file.readlines()
            # We leave a blank space in the end as we want to search exactly for this mount points
            boot_mount_point = self.dest_dir + "/boot "
            root_mount_point = self.dest_dir + " "
            for line in mounts:
                if " xfs " in line:
                    if boot_mount_point in line:
                        xfs_boot = True
                    elif root_mount_point in line:
                        xfs_root = True
            if xfs_boot:
                boot_mount_point = boot_mount_point.rstrip()
                subprocess.check_call(["xfs_freeze", "-f", boot_mount_point])
                subprocess.check_call(["xfs_freeze", "-u", boot_mount_point])
            if xfs_root:
                subprocess.check_call(["xfs_freeze", "-f", self.dest_dir])
                subprocess.check_call(["xfs_freeze", "-u", self.dest_dir])
        except subprocess.CalledProcessError as process_error:
            logging.warning(_("Can't freeze/unfreeze xfs system"))
            logging.warning(process_error)

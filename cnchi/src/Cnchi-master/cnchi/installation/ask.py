#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  ask.py
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

""" Asks which type of installation the user wants to perform """

import os
import sys

# When testing, no _() is available
try:
    _("")
except NameError as err:
    def _(message):
        return message

if __name__ == '__main__':
    # Insert the parent directory at the front of the path.
    # This is used only when we want to test this screen
    BASE_DIR = os.path.dirname(__file__) or '.'
    PARENT_DIR = os.path.join(BASE_DIR, '..')
    sys.path.insert(0, PARENT_DIR)

import bootinfo
import logging

from gtkbasebox import GtkBaseBox
import misc.misc as misc


def check_alongside_disk_layout():
    """ Alongside can only work if user has followed the recommended
        BIOS-Based Disk-Partition Configurations shown in
        http://technet.microsoft.com/en-us/library/dd744364(v=ws.10).aspx """

    # TODO: Add more scenarios where alongside could work

    partitions = misc.get_partitions()
    # logging.debug(partitions)
    extended = False
    for partition in partitions:
        if misc.is_partition_extended(partition):
            extended = True

    if extended:
        return False

    # We just seek for sda partitions
    partitions_sda = []
    for partition in partitions:
        if "sda" in partition:
            partitions_sda.append(partition)

    # There's no extended partition, so all partitions must be primary
    if len(partitions_sda) < 4:
        return True

    return False


class InstallationAsk(GtkBaseBox):
    def __init__(self, params, prev_page="keymap", next_page=None):
        super().__init__(self, params, "ask", prev_page, next_page)

        data_dir = self.settings.get("data")

        partitioner_dir = os.path.join(data_dir, "images", "partitioner", "small")

        image = self.ui.get_object("automatic_image")
        path = os.path.join(partitioner_dir, "automatic.png")
        image.set_from_file(path)

        # image = self.ui.get_object("alongside_image")
        # path = os.path.join(partitioner_dir, "alongside.png")
        # image.set_from_file(path)

        image = self.ui.get_object("advanced_image")
        path = os.path.join(partitioner_dir, "advanced.png")
        image.set_from_file(path)

        self.other_oses = []

        # DISABLE ALONGSIDE INSTALLATION. IT'S NOT READY YET
        # enable_alongside = self.check_alongside()
        enable_alongside = False
        self.settings.set('enable_alongside', enable_alongside)
        '''
        if enable_alongside:
            msg = _("Cnchi will enable the 'alongside' installation mode.")
        else:
            msg = _("Cnchi will NOT enable the 'alongside' installation mode.")
        logging.debug(msg)
        '''

        # By default, select automatic installation
        self.next_page = "installation_automatic"

    def check_alongside(self):
        """ Check if alongside installation type must be enabled.
        Alongside only works when Windows is installed on sda  """

        enable_alongside = False

        # FIXME: Alongside does not work in UEFI systems
        if os.path.exists("/sys/firmware/efi"):
            msg = _("The 'alongside' installation mode does not work in UEFI systems")
            logging.debug(msg)
            enable_alongside = False
        else:
            oses = bootinfo.get_os_dict()
            self.other_oses = []
            for key in oses:
                # We only check the first hard disk
                if "sda" in key and oses[key] not in ["unknown", "Swap", "Data or Swap"] and oses[key] not in self.other_oses:
                    self.other_oses.append(oses[key])

            if len(self.other_oses) > 0:
                for detected_os in self.other_oses:
                    if "windows" in detected_os.lower():
                        logging.debug(_("Windows(tm) OS detected."))
                        enable_alongside = True
                if not enable_alongside:
                    logging.debug(_("Windows(tm) OS not detected."))
                    enable_alongside = False
            else:
                logging.debug(_("Can't detect any OS in device sda."))
                enable_alongside = False

            if not check_alongside_disk_layout():
                logging.debug(_("Unsuported disk layout for the 'alongside' installation mode"))
                enable_alongside = False

        return enable_alongside

    def enable_automatic_options(self, status):
        """ Enables or disables automatic installation options """
        names = [
            "encrypt_checkbutton",
            "encrypt_label",
            "lvm_checkbutton",
            "lvm_label",
            "home_checkbutton",
            "home_label"]

        for name in names:
            obj = self.ui.get_object(name)
            obj.set_sensitive(status)

    def prepare(self, direction):
        """ Prepares screen """
        self.translate_ui()
        self.show_all()

        if not self.settings.get('enable_alongside'):
            self.hide_alongside_option()

    def hide_alongside_option(self):
        """ Hides alongisde widgets """
        widgets = [
            "alongside_radiobutton",
            "alongside_description",
            "alongside_image"]

        for name in widgets:
            widget = self.ui.get_object(name)
            if widget is not None:
                widget.hide()

    def get_os_list_str(self):
        os_str = ""
        len_other_oses = len(self.other_oses)
        if len_other_oses > 0:
            if len_other_oses > 1:
                if len_other_oses == 2:
                    os_str = _(" and ").join(self.other_oses)
                else:
                    os_str = ", ".join(self.other_oses)
            else:
                os_str = self.other_oses[0]

        # Truncate string if it's too large
        if len(os_str) > 40:
            os_str = os_str[:40] + "..."

        return os_str

    def translate_ui(self):
        """ Translates screen before showing it """
        self.header.set_subtitle(_("Installation Type"))

        self.forward_button.set_always_show_image(True)
        self.forward_button.set_sensitive(True)

        description_style = '<span style="italic">{0}</span>'
        bold_style = '<span weight="bold">{0}</span>'

        oses_str = self.get_os_list_str()

        # Automatic Install
        radio = self.ui.get_object("automatic_radiobutton")
        if len(oses_str) > 0:
            txt = _("Replace {0} with Apricity OS").format(oses_str)
        else:
            txt = _("Erase disk and install Apricity OS")
        radio.set_label(txt)
        radio.set_name('auto_radio_btn')

        label = self.ui.get_object("automatic_description")
        txt = _("Warning: This will erase ALL data on your disk")
        #txt = description_style.format(txt)
        label.set_text(txt)
        label.set_line_wrap(True)
        label.set_name("automatic_desc")

        button = self.ui.get_object("encrypt_checkbutton")
        txt = _("Encrypt this installation for increased security")
        button.set_label(txt)
        button.set_name("enc_btn")

        label = self.ui.get_object("encrypt_label")
        txt = _("You will be asked to create an encryption password in the next step")
        #txt = description_style.format(txt)
        label.set_text(txt)
        label.set_name("enc_label")

        button = self.ui.get_object("lvm_checkbutton")
        txt = _("Use LVM with this installation")
        button.set_label(txt)
        button.set_name("lvm_btn")

        label = self.ui.get_object("lvm_label")
        txt = _("This will setup LVM and allow you to easily manage partitions and create snapshots")
        #txt = description_style.format(txt)
        label.set_text(txt)
        label.set_name("lvm_label")

        button = self.ui.get_object("home_checkbutton")
        txt = _("Set your Home in a different partition/volume")
        button.set_label(txt)
        button.set_name("home_btn")

        label = self.ui.get_object("home_label")
        txt = _("This will setup your /home directory in a different partition or volume")
        #txt = description_style.format(txt)
        label.set_text(txt)
        label.set_name("home_label")

        # Alongside Install (For now, only works with Windows)
        # if len(oses_str) > 0:
        #     txt = _("Install Apricity alongside {0}").format(oses_str)
        #     radio = self.ui.get_object("alongside_radiobutton")
        #     radio.set_label(txt)
        #
        #     label = self.ui.get_object("alongside_description")
        #     txt = _("Installs Apricity without removing {0}").format(oses_str)
        #     txt = description_style.format(txt)
        #     label.set_markup(txt)
        #     label.set_line_wrap(True)
        #
        #     intro_txt = _("This computer has {0} installed.").format(oses_str)
        #     intro_txt = intro_txt + "\n" + _("What do you want to do?")
        # else:
        intro_txt = _("How would you like to proceed?")

        intro_label = self.ui.get_object("introduction")
        #intro_txt = bold_style.format(intro_txt)
        intro_label.set_text(intro_txt)
        intro_label.set_name("intro_label")

        # Advanced Install
        radio = self.ui.get_object("advanced_radiobutton")
        radio.set_label(_("Choose exactly where Apricity OS should be installed"))
        radio.set_name("advanced_radio_btn")

        label = self.ui.get_object("advanced_description")
        txt = _("Edit partition table and choose mount points")
        #txt = description_style.format(txt)
        label.set_text(txt)
        label.set_line_wrap(True)
        label.set_name("adv_desc_label")

    def store_values(self):
        """ Store selected values """
        check = self.ui.get_object("encrypt_checkbutton")
        use_luks = check.get_active()

        check = self.ui.get_object("lvm_checkbutton")
        use_lvm = check.get_active()

        check = self.ui.get_object("home_checkbutton")
        use_home = check.get_active()

        if self.next_page == "installation_automatic":
            self.settings.set('use_lvm', use_lvm)
            self.settings.set('use_luks', use_luks)
            self.settings.set('use_luks_in_root', True)
            self.settings.set('luks_root_volume', "cryptApricity")
            self.settings.set('use_home', use_home)
        else:
            # Set defaults. We don't know these yet.
            self.settings.set('use_lvm', False)
            self.settings.set('use_luks', False)
            self.settings.set('use_luks_in_root', False)
            self.settings.set('luks_root_volume', "")
            self.settings.set('use_home', False)

        if self.settings.get('use_luks'):
            logging.info(_("Apricity OS installation will be encrypted using LUKS"))

        if self.settings.get('use_lvm'):
            logging.info(_("Apricity OS will be installed using LVM volumes"))
            if self.settings.get('use_home'):
                logging.info(_("Apricity OS will be installed using a separate /home volume."))
        elif self.settings.get('use_home'):
            logging.info(_("Apricity OS will be installed using a separate /home partition."))

        if self.next_page == "installation_alongside":
            self.settings.set('partition_mode', 'alongside')
        elif self.next_page == "installation_advanced":
            self.settings.set('partition_mode', 'advanced')
        elif self.next_page == "installation_automatic":
            self.settings.set('partition_mode', 'automatic')

        return True

    def get_next_page(self):
        return self.next_page

    def on_automatic_radiobutton_toggled(self, widget):
        """ Automatic selected, enable all options """
        if widget.get_active():
            self.next_page = "installation_automatic"
            self.enable_automatic_options(True)

    def on_alongside_radiobutton_toggled(self, widget):
        """ Alongside selected, disable all automatic options """
        if widget.get_active():
            self.next_page = "installation_alongside"
            self.enable_automatic_options(False)

    def on_advanced_radiobutton_toggled(self, widget):
        """ Advanced selected, disable all automatic options """
        if widget.get_active():
            self.next_page = "installation_advanced"
            self.enable_automatic_options(False)


if __name__ == '__main__':
    from test_screen import _, run

    run('InstallationAsk')

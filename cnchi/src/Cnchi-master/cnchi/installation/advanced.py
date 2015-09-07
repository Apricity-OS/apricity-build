#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  advanced.py
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

""" Installation advanced module. Custom partition screen """

from gi.repository import Gtk, Gdk
import subprocess
import os
import logging

# import blivet

import misc.misc as misc
import misc.validation as validation

import parted3.partition_module as pm
import parted3.fs_module as fs
import parted3.lvm as lvm
import parted3.used_space as used_space

from installation import process as installation_process
import show_message as show

from gtkbasebox import GtkBaseBox

COL_PATH = 0
COL_FS = 1
COL_MOUNT_POINT = 2
COL_LABEL = 3
COL_FORMAT_ACTIVE = 4
COL_FORMAT_VISIBLE = 5
COL_SIZE = 6
COL_USED = 7
COL_PARTITION_PATH = 8
COL_FLAGS = 9
COL_PARTITION_TYPE = 10
COL_FORMAT_SENSITIVE = 11
COL_SSD_ACTIVE = 12
COL_SSD_VISIBLE = 13
COL_SSD_SENSITIVE = 14
COL_ENCRYPTED = 15


class InstallationAdvanced(GtkBaseBox):
    """ Installation advanced class. Custom partitioning. """

    def __init__(self, params, prev_page="installation_ask", next_page="user_info"):
        # Call base class
        super().__init__(self, params, "advanced", prev_page, next_page)

        # Init class vars

        self.blvm = False

        self.lv_partitions = []
        self.disks_changed = []

        self.check_ok_once = False

        # Store here all LUKS options for each partition (if any)
        # stores tuple (use_luks, vol_name, password)
        # uses partition uid as index
        self.luks_options = {}

        # Store here LUKS options for current partition (create/edit dialog)
        # (they will finally be stored in luks_options if user selects ok in create/edit partition dialog)
        # stores tuple (use_luks, vol_name, password)
        self.tmp_luks_options = (False, "", "")

        self.first_time_in_fill_partition_list = True

        self.orig_label_dic = {}
        self.orig_part_dic = {}

        # stage_opts holds info about newly created partitions (it's like a todo list)
        # format is tuple (is_new, label, mount_point, fs, format)
        # see its usage in listing, creating, and deleting partitions
        # uses partition uid as index
        self.stage_opts = {}

        # What's this?
        self.used_dic = {}

        # Holds partitions that exist now but are going to be deleted
        self.to_be_deleted = []

        # We will store our devices here
        self.disks = None

        # We will store if our device is SSD or not
        self.ssd = {}

        self.show_changes_grid = None

        # Initialize some attributes
        self.process = None
        self.diskdic = {}

        # Store here ALL partitions from ALL devices
        self.all_partitions = []

        # Init GUI elements

        # Load create and edit partition dialogs
        self.create_partition_dialog = self.ui.get_object('create_partition_dialog')
        self.edit_partition_dialog = self.ui.get_object('edit_partition_dialog')

        # setup_luks freezes our UI completely. It's no use to try to use a progressbar
        # self.advanced_progressbar = self.ui.get_object('advanced_progressbar')

        # Initialize our create partition dialog filesystems' combo.
        combo = self.ui.get_object('create_partition_use_combo')
        combo.remove_all()
        for fs_name in sorted(fs.NAMES):
            combo.append_text(fs_name)
        combo.set_wrap_width(2)

        # Initialize our edit partition dialog filesystems' combo.
        combo = self.ui.get_object('edit_partition_use_combo')
        combo.remove_all()
        for fs_name in sorted(fs.NAMES):
            combo.append_text(fs_name)
        combo.set_wrap_width(2)

        # Initialize partition_types_combo
        combo = self.ui.get_object('partition_types_combo')
        combo.remove_all()
        combo.append_text("msdos (MBR)")
        combo.append_text("GUID Partition Table (GPT)")

        # Automatically select first entry
        self.select_first_combobox_item(combo)

        # Initialize our create and edit partition dialog mount points' combo.
        mount_combos = [self.ui.get_object('create_partition_mount_combo'),
                        self.ui.get_object('edit_partition_mount_combo')]

        for combo in mount_combos:
            combo.remove_all()
            for mount_point in fs.COMMON_MOUNT_POINTS:
                combo.append_text(mount_point)

        self.bootloader = "grub2"
        self.bootloader_device = ""
        self.bootloader_entry = self.ui.get_object('bootloader_entry')
        self.bootloader_device_entry = self.ui.get_object('bootloader_device_entry')
        self.bootloader_devices = {}

        # Initialise our partition list tree view
        self.partition_list = self.ui.get_object('partition_list_treeview')
        self.partition_list_store = None
        self.prepare_partition_list()

        self.partition_list.set_hexpand(True)

        # Get encryption (LUKS) options dialog
        self.luks_dialog = self.ui.get_object('luks_dialog')

        switch = self.ui.get_object('luks_use_luks_switch')
        switch.connect('notify::active', self.on_luks_use_luks_switch_activate)

        # Connect changing selection in the partition list treeview
        select = self.partition_list.get_selection()
        select.connect("changed", self.on_partition_list_treeview_selection_changed)

        # Assign images to buttons
        btns = [
            ("partition_button_undo", "edit-undo-symbolic"),
            ("partition_button_new", "list-add-symbolic"),
            ("partition_button_delete", "list-remove-symbolic"),
            ("partition_button_edit", "system-run-symbolic"),
            ("partition_button_new_label", "edit-clear-all-symbolic"),
            ("changelist_cancelbutton", "dialog-cancel"),
            ("changelist_okbutton", "dialog-apply"),
            ("create_table_dialog_cancel", "dialog-cancel"),
            ("create_table_dialog_ok", "dialog-apply"),
            ("edit_partition_cancel", "dialog-cancel"),
            ("edit_partition_ok", "dialog-apply"),
            ("create_partition_cancel", "dialog-cancel"),
            ("create_partition_ok", "dialog-apply"),
            ("luks_cancel_button", "dialog-cancel"),
            ("luks_ok_button", "dialog-apply")]

        for grp in btns:
            (btn_id, icon) = grp
            image = Gtk.Image.new_from_icon_name(icon, Gtk.IconSize.BUTTON)
            btn = self.ui.get_object(btn_id)
            btn.set_always_show_image(True)
            btn.set_image(image)

    def gen_partition_uid(self, partition=None, path=None):
        """ Function to generate uid by partition object or path """
        if path and not partition:
            if "free" in path:
                return None
            # Search for partition with path "path"
            for part in self.all_partitions:
                if "/dev/mapper" not in part:
                    for i in part:
                        if part[i].path == path:
                            partition = part[i]
        try:
            dev_path = partition.disk.device.path
        except Exception:
            dev_path = path

        if partition:
            ends = partition.geometry.end
            starts = partition.geometry.start
        else:
            ends = 'none'
            starts = 'none'

        uid = dev_path + str(starts) + str(ends)
        return uid

    def check_buttons(self, selection):
        """ Activates/deactivates our buttons depending on which is selected in the
            partition treeview """

        model, tree_iter = selection.get_selected()

        path = None
        if tree_iter is not None:
            path = model[tree_iter][COL_PATH]

        if path is not None:
            button = {"undo": self.ui.get_object('partition_button_undo'),
                      "new": self.ui.get_object('partition_button_new'),
                      "delete": self.ui.get_object('partition_button_delete'),
                      "edit": self.ui.get_object('partition_button_edit'),
                      "new_label": self.ui.get_object('partition_button_new_label')}

            for key in button:
                button[key].set_sensitive(False)

            if self.stage_opts:
                button["undo"].set_sensitive(True)

            if path == _("free space"):
                button["new"].set_sensitive(True)
            else:
                disks = pm.get_devices()
                if (path not in disks and 'dev/mapper' not in path) or ('dev/mapper' in path and '-' in path):
                    # A partition is selected
                    diskobj = None
                    for i in self.all_partitions:
                        if path in i and '/mapper' not in path and '/' in path:
                            diskobj = i[path].disk.device.path
                    if diskobj and model[tree_iter][COL_FS] == 'extended' and self.diskdic[diskobj]['has_logical']:
                        # It's an extended partition and has logical ones in it,
                        # so it can't be edited or deleted until the logical ones are deleted first.
                        button["delete"].set_sensitive(False)
                        button["edit"].set_sensitive(False)
                    else:
                        if '/mapper' in path:
                            button["delete"].set_sensitive(False)
                        else:
                            button["delete"].set_sensitive(True)

                        button["edit"].set_sensitive(True)
                else:
                    # A drive (disk) is selected
                    button["new_label"].set_sensitive(True)

    def fill_bootloader_device_entry(self):
        """ Get all devices where we can put our bootloader. Avoiding partitions """

        self.bootloader_device_entry.remove_all()
        self.bootloader_devices.clear()

        # Just call get_devices once
        if self.disks is None:
            self.disks = pm.get_devices()

        for path in sorted(self.disks):
            (disk, result) = self.disks[path]
            if disk is not None:
                dev = disk.device
                # Avoid cdrom and any raid, lvm volumes or encryptfs
                if not dev.path.startswith("/dev/sr") and not dev.path.startswith("/dev/mapper"):
                    # Hard drives measure themselves assuming kilo=1000, mega=1mil, etc
                    size_in_gigabytes = int((dev.length * dev.sectorSize) / 1000000000)
                    line = '{0} [{1} GB] ({2})'.format(dev.model, size_in_gigabytes, dev.path)
                    self.bootloader_device_entry.append_text(line)
                    self.bootloader_devices[line] = dev.path

        if not self.select_bootdevice(self.bootloader_device_entry, self.bootloader_device):
            # Automatically select first entry
            self.select_first_combobox_item(self.bootloader_device_entry)

    def select_bootdevice(self, combobox, value):
        model = combobox.get_model()
        combo_iter = model.get_iter_first()
        index = 0
        found = False
        while combo_iter is not None and not found:
            if value.lower() in model[combo_iter][0].lower():
                combobox.set_active_iter(combo_iter)
                combo_iter = None
                found = True
            else:
                index += 1
                combo_iter = model.iter_next(combo_iter)
        return found

    def fill_bootloader_entry(self):
        """ Put the bootloaders for the user to choose """
        self.bootloader_entry.remove_all()

        if os.path.exists('/sys/firmware/efi'):
            self.bootloader_entry.append_text("Grub2")
            self.bootloader_entry.append_text("Gummiboot")
            if not self.select_combobox_value(self.bootloader_entry, self.bootloader):
                # Automatically select first entry
                self.bootloader_entry.set_active(0)
            self.bootloader_entry.show()
        else:
            self.bootloader_entry.hide()
            widget_ids = ["bootloader_label", "bootloader_device_label"]
            for widget_id in widget_ids:
                widget = self.ui.get_object(widget_id)
                widget.hide()

    def select_combobox_value(self, combobox, value):
        model = combobox.get_model()
        combo_iter = model.get_iter(0)
        index = 0
        found = False
        while combo_iter is not None and not found:
            if value.lower() == model[combo_iter][0].lower():
                combobox.set_active_iter(combo_iter)
                combo_iter = None
                found = True
            else:
                index += 1
                combo_iter = model.iter_next(combo_iter)
        return found

    def on_bootloader_device_check_toggled(self, checkbox):
        status = checkbox.get_active()

        widget_ids = [
            "bootloader_device_entry", "bootloader_entry",
            "bootloader_label", "bootloader_device_label"]

        for widget_id in widget_ids:
            widget = self.ui.get_object(widget_id)
            widget.set_sensitive(status)

    @staticmethod
    def select_first_combobox_item(combobox):
        """ Automatically select first entry """
        tree_model = combobox.get_model()
        tree_iter = tree_model.get_iter_first()
        combobox.set_active_iter(tree_iter)

    def on_bootloader_device_entry_changed(self, widget):
        """ Get new selected bootloader device """
        line = self.bootloader_device_entry.get_active_text()
        if line is not None:
            self.bootloader_device = self.bootloader_devices[line]

    def on_bootloader_entry_changed(self, widget):
        """ Get new selected bootloader """
        line = self.bootloader_entry.get_active_text()
        if line is not None:
            self.bootloader = line.lower()
            self.check_mount_points()

    def prepare_partition_list(self):
        """ Create columns for our treeview """
        render_text = Gtk.CellRendererText()

        col = Gtk.TreeViewColumn(_("Device"), render_text, text=COL_PATH)
        self.partition_list.append_column(col)

        col = Gtk.TreeViewColumn(_("Type"), render_text, text=COL_FS)
        self.partition_list.append_column(col)

        col = Gtk.TreeViewColumn(_("Mount point"), render_text, text=COL_MOUNT_POINT)
        self.partition_list.append_column(col)

        col = Gtk.TreeViewColumn(_("Label"), render_text, text=COL_LABEL)
        self.partition_list.append_column(col)

        format_toggle = Gtk.CellRendererToggle()
        format_toggle.connect("toggled", self.on_format_cell_toggled)

        col = Gtk.TreeViewColumn(
            _("Format"),
            format_toggle,
            active=COL_FORMAT_ACTIVE,
            visible=COL_FORMAT_VISIBLE,
            sensitive=COL_FORMAT_SENSITIVE)
        self.partition_list.append_column(col)

        col = Gtk.TreeViewColumn(_("Size"), render_text, text=COL_SIZE)
        self.partition_list.append_column(col)

        col = Gtk.TreeViewColumn(_("Used"), render_text, text=COL_USED)
        self.partition_list.append_column(col)

        col = Gtk.TreeViewColumn(_("Flags"), render_text, text=COL_FLAGS)
        self.partition_list.append_column(col)

        ssd_toggle = Gtk.CellRendererToggle()
        ssd_toggle.connect("toggled", self.on_ssd_cell_toggled)

        col = Gtk.TreeViewColumn(
            _("SSD"), ssd_toggle, active=COL_SSD_ACTIVE, visible=COL_SSD_VISIBLE, sensitive=COL_SSD_SENSITIVE)
        self.partition_list.append_column(col)

    @staticmethod
    def get_size(length, sector_size):
        """ Helper function to get a disk/partition size in human format """
        size = length * sector_size
        size_txt = "{0}b".format(size)

        if size >= 1000000000:
            size /= 1000000000
            size_txt = "{0:.0f}G".format(size)
        elif size >= 1000000:
            size /= 1000000
            size_txt = "{0:.0f}M".format(size)
        elif size >= 1000:
            size /= 1000
            size_txt = "{0:.0f}K".format(size)

        return size_txt

    def fill_partition_list(self):
        """ Fill the partition list with all the data. """

        # We will store our data model in 'partition_list_store'
        if self.partition_list_store is not None:
            self.partition_list_store.clear()

        self.partition_list_store = Gtk.TreeStore(
            str, str, str, str, bool, bool, str, str, str, str, int, bool, bool, bool, bool, bool)

        # Be sure to call get_devices once
        if self.disks is None:
            self.disks = pm.get_devices()

        self.diskdic = {}
        self.all_partitions = []
        self.lv_partitions = []
        self.diskdic['mounts'] = []

        # Put all volumes (lvm) info in our model
        volume_groups = lvm.get_volume_groups()
        if volume_groups:
            for volume_group in volume_groups:
                is_ssd = False
                logical_volumes = lvm.get_logical_volumes(volume_group)
                if not logical_volumes:
                    continue
                row = [volume_group, "", "", "", False, False, "", "", "", "", 0, False, is_ssd, False, False, False]
                lvparent = self.partition_list_store.append(None, row)
                for logical_volume in logical_volumes:
                    fmt_enable = True
                    fmt_active = False
                    label = ""
                    mount_point = ""
                    formatable = True

                    partition_path = "/dev/mapper/{0}-{1}".format(volume_group, logical_volume)
                    self.all_partitions.append(partition_path)
                    self.lv_partitions.append(partition_path)

                    uid = self.gen_partition_uid(path=partition_path)

                    if fs.get_type(partition_path):
                        fs_type = fs.get_type(partition_path)
                    elif used_space.is_btrfs(partition_path):
                        # kludge, btrfs not being detected...
                        fs_type = 'btrfs'
                    else:
                        # Say unknown if we can't detect fs type instead of assumming btrfs
                        fs_type = 'unknown'

                    if uid in self.stage_opts:
                        (is_new, label, mount_point, fs_type, fmt_active) = self.stage_opts[uid]

                    info = fs.get_info(partition_path)
                    if 'LABEL' in info:
                        label = info['LABEL']

                    if mount_point:
                        self.diskdic['mounts'].append(mount_point)

                    # Do not show swap version, only the 'swap' word
                    if 'swap' in fs_type:
                        fs_type = 'swap'

                    row = [partition_path, fs_type, mount_point, label, fmt_active, formatable, '', '',
                           partition_path, "", 0, fmt_enable, is_ssd, False, False, False]

                    self.partition_list_store.append(lvparent, row)

                    if self.first_time_in_fill_partition_list:
                        self.orig_part_dic[partition_path] = uid
                        self.orig_label_dic[partition_path] = label

        # Fill our model with the rest of devices (non LVM)
        for disk_path in sorted(self.disks):
            if '/dev/mapper/arch_' in disk_path:
                # Already added
                continue

            self.diskdic[disk_path] = {}
            self.diskdic[disk_path]['has_logical'] = False
            self.diskdic[disk_path]['has_extended'] = False

            if disk_path not in self.ssd:
                self.ssd[disk_path] = fs.is_ssd(disk_path)

            is_ssd = self.ssd[disk_path]

            (disk, result) = self.disks[disk_path]

            if disk is None:
                # Disk without a partition table
                row = [disk_path, "", "", "", False, False, "", "", "", "", 0, False, is_ssd, False, False, False]
                self.partition_list_store.append(None, row)
            elif '/dev/mapper/' in disk_path:
                # Already added
                continue
            else:
                dev = disk.device

                # Get device size
                size_txt = self.get_size(dev.length, dev.sectorSize)

                # Append the device info to our model
                row = [dev.path, "", "", "", False, False, size_txt, "", "", "", 0, False, is_ssd, True, True, False]
                disk_parent = self.partition_list_store.append(None, row)

                extended_parent = None

                # Create a list of partitions for this device (/dev/sda for example)
                partitions = pm.get_partitions(disk)
                self.all_partitions.append(partitions)
                partition_list = pm.order_partitions(partitions)

                # Append all partitions to our model
                for partition_path in partition_list:
                    # Get partition size
                    partition = partitions[partition_path]
                    size_txt = self.get_size(partition.geometry.length, dev.sectorSize)
                    fmt_active = False
                    label = ""
                    mount_point = ""
                    used = ""
                    formatable = True

                    path = partition.path

                    # Skip lvm, LUKS, cdrom, ...
                    if '/dev/mapper' in path or 'sr0' in path:
                        continue

                    # Get filesystem
                    if partition.fileSystem and partition.fileSystem.type:
                        fs_type = partition.fileSystem.type
                    # Check if its free space before trying to get the filesystem with blkid.
                    elif 'free' in partition_path:
                        fs_type = _("none")
                    elif fs.get_type(path):
                        fs_type = fs.get_type(path)
                    else:
                        # Unknown filesystem
                        fs_type = '?'

                    # Nothing should be mounted at this point

                    if partition.type == pm.PARTITION_EXTENDED:
                        formatable = False
                        self.diskdic[disk_path]['has_extended'] = True
                    elif partition.type == pm.PARTITION_LOGICAL:
                        formatable = True
                        self.diskdic[disk_path]['has_logical'] = True

                    if partition.type in (pm.PARTITION_FREESPACE, pm.PARTITION_FREESPACE_EXTENDED):
                        # Show 'free space' instead of /dev/sda-1
                        path = _("free space")
                        formatable = False
                    # else:
                    #    # Get partition flags
                    #    flags = pm.get_flags(partition)

                    uid = self.gen_partition_uid(partition=partition)
                    if uid in self.stage_opts:
                        (is_new, label, mount_point, fs_type, fmt_active) = self.stage_opts[uid]
                        fmt_enable = not is_new
                        if mount_point == "/":
                            fmt_enable = False
                    else:
                        fmt_enable = True
                        if _("free space") not in path:
                            if self.first_time_in_fill_partition_list:
                                if mount_point:
                                    used = pm.get_used_space(partition)
                                else:
                                    used = used_space.get_used_space(partition_path,
                                                                     fs_type) * partition.geometry.length
                                    used = self.get_size(used, dev.sectorSize)
                                self.used_dic[(disk_path, partition.geometry.start)] = used
                            else:
                                if (disk_path, partition.geometry.start) in self.used_dic:
                                    used = self.used_dic[(disk_path, partition.geometry.start)]
                                else:
                                    used = '0b'
                            info = fs.get_info(partition_path)
                            if 'LABEL' in info:
                                label = info['LABEL']

                    if mount_point:
                        self.diskdic['mounts'].append(mount_point)

                    if partition.type == pm.PARTITION_EXTENDED:
                        # Show 'extended' in file system type column
                        fs_type = 'extended'

                    # Do not show swap version, only the 'swap' word
                    if 'swap' in fs_type:
                        fs_type = 'swap'

                    row = [path, fs_type, mount_point, label, fmt_active, formatable, size_txt, used,
                           partition_path, "", partition.type, fmt_enable, False, False, False, False]

                    if partition.type in (pm.PARTITION_LOGICAL, pm.PARTITION_FREESPACE_EXTENDED):
                        # Our parent (in the treeview) will be the extended partition we're in, not the disk
                        parent = extended_parent
                    else:
                        # Our parent (in the treeview) will be the disk we're in
                        parent = disk_parent

                    tree_iter = self.partition_list_store.append(parent, row)

                    # If we're an extended partition, all the logical partitions
                    # that follow will be shown as children of this one
                    if partition.type == pm.PARTITION_EXTENDED:
                        extended_parent = tree_iter

                    if self.first_time_in_fill_partition_list:
                        self.orig_part_dic[partition.path] = self.gen_partition_uid(partition=partition)
                        self.orig_label_dic[partition.path] = label

        self.first_time_in_fill_partition_list = False

        # Assign our new model to our treeview
        self.partition_list.set_model(self.partition_list_store)
        self.partition_list.expand_all()

    def on_format_cell_toggled(self, widget, path):
        """ Mark a partition to be formatted """
        self.partition_list_store[path][COL_FORMAT_ACTIVE] = not self.partition_list_store[path][COL_FORMAT_ACTIVE]

        partition_path = self.partition_list_store[path][COL_PATH]
        uid = self.gen_partition_uid(path=partition_path)
        # As it's not a new partition, we set it to False
        self.stage_opts[uid] = (
            False,
            self.partition_list_store[path][COL_LABEL],
            self.partition_list_store[path][COL_MOUNT_POINT],
            self.partition_list_store[path][COL_FS],
            self.partition_list_store[path][COL_FORMAT_ACTIVE])

    def on_ssd_cell_toggled(self, widget, path):
        """ Mark disk as ssd """
        self.partition_list_store[path][COL_SSD_ACTIVE] = not self.partition_list_store[path][COL_SSD_ACTIVE]
        disk_path = self.partition_list_store[path][COL_PATH]
        self.ssd[disk_path] = self.partition_list_store[path][COL_SSD_ACTIVE]

    def on_luks_password_changed(self, widget):
        """ User has introduced new information. Check it here. """
        luks_password_entry = self.ui.get_object('luks_password_entry')
        luks_password_confirm_entry = self.ui.get_object('luks_password_confirm_entry')
        luks_password_confirm_image = self.ui.get_object('luks_password_confirm_image')
        luks_password_status_label = self.ui.get_object('luks_password_status_label')
        luks_password_strength = self.ui.get_object('luks_password_strength')

        if widget == luks_password_entry or widget == luks_password_confirm_entry:
            validation.check_password(
                luks_password_entry,
                luks_password_confirm_entry,
                luks_password_confirm_image,
                luks_password_status_label,
                luks_password_strength)

    def on_partition_list_edit_activate(self, button):
        """ The user wants to edit a partition """
        selection = self.partition_list.get_selection()
        if not selection:
            return

        model, tree_iter = selection.get_selected()
        if tree_iter is None:
            return

        # Get selected row's data
        row = model[tree_iter]

        # Can't edit a partition that uses a LVM filesystem type
        if "lvm2" in row[COL_FS].lower():
            logging.warning(_("Can't edit a partition with LVM filesystem type"))
            return

        # Fill partition dialog with correct data

        # Select the fs in dialog combobox
        combo = self.ui.get_object('edit_partition_use_combo')
        combo_model = combo.get_model()
        combo_iter = combo_model.get_iter_first()
        while combo_iter is not None:
            combo_row = combo_model[combo_iter]
            if combo_row[0] and combo_row[0] in row[COL_FS]:
                combo.set_active_iter(combo_iter)
                combo_iter = None
            else:
                combo_iter = combo_model.iter_next(combo_iter)

        # Set the mount point in dialog combobox
        mount_combo_entry = self.ui.get_object('edit_partition_mount_combo_entry')
        mount_combo_entry.set_text(row[COL_MOUNT_POINT])

        # Set label entry
        label_entry = self.ui.get_object('edit_partition_label_entry')
        label_entry.set_text(row[COL_LABEL])

        # Must format?
        format_check = self.ui.get_object('edit_partition_format_check')
        format_check.set_active(row[COL_FORMAT_ACTIVE])
        format_check.set_sensitive(row[COL_FORMAT_SENSITIVE])

        # Be sure to just call get_devices once
        if self.disks is None:
            self.disks = pm.get_devices()

        # Get disk path
        disk_path = self.get_disk_path_from_selection(model, tree_iter)

        '''
        try:
            disk, result = self.disks[disk_path]
        except Exception:
            disk = None
        '''

        uid = self.gen_partition_uid(path=row[COL_PARTITION_PATH])

        # Get LUKS info for the encryption properties dialog
        if uid in self.luks_options.keys():
            self.tmp_luks_options = self.luks_options[uid]
        else:
            self.tmp_luks_options = (False, "", "")

        # Dialog windows should be set transient for the main application window they were spawned from.
        self.edit_partition_dialog.set_transient_for(self.get_toplevel())

        # Show edit partition dialog
        response = self.edit_partition_dialog.run()

        if response == Gtk.ResponseType.OK:
            new_mount = mount_combo_entry.get_text().strip()

            if new_mount in self.diskdic['mounts'] and new_mount != row[COL_MOUNT_POINT]:
                show.warning(self.get_toplevel(), _("Can't use same mount point twice."))
            elif new_mount == "/" and not format_check.get_active():
                show.warning(self.get_toplevel(), _('Root partition must be formatted.'))
            else:
                if row[COL_MOUNT_POINT]:
                    self.diskdic['mounts'].remove(row[COL_MOUNT_POINT])

                new_label = label_entry.get_text()
                new_fs = combo.get_active_text()
                new_format = format_check.get_active()

                if uid in self.stage_opts:
                    is_new = self.stage_opts[uid][0]
                else:
                    is_new = False

                if new_fs == 'swap':
                    new_mount = 'swap'

                self.stage_opts[uid] = (is_new, new_label, new_mount, new_fs, new_format)
                self.luks_options[uid] = self.tmp_luks_options

                if new_mount == "/":
                    # Set if we'll be using LUKS in the root partition (for process.py to know)
                    self.settings.set('use_luks_in_root', self.tmp_luks_options[0])
                    self.settings.set('luks_root_volume', self.tmp_luks_options[1])

        self.edit_partition_dialog.hide()

        # Update the partition list treeview
        self.update_view()

    def update_view(self):
        """ Reloads widgets contents """
        self.fill_partition_list()
        self.fill_bootloader_device_entry()
        self.fill_bootloader_entry()
        # Check if correct mount points are already defined, so we can proceed with installation
        self.check_mount_points()

    @staticmethod
    def get_disk_path_from_selection(model, tree_iter):
        """ Helper function that returns the disk path where the selected partition is in """
        if tree_iter is not None and model is not None:
            row = model[tree_iter]
            # partition_path = row[COL_PARTITION_PATH]

            # Get partition type from the user selection
            part_type = row[COL_PARTITION_TYPE]

            # Get our parent drive
            parent_iter = model.iter_parent(tree_iter)

            if part_type == pm.PARTITION_LOGICAL:
                # If we are a logical partition, our drive won't be our father
                # but our grandpa (we have to skip the extended partition we're in)
                parent_iter = model.iter_parent(parent_iter)

            return model[parent_iter][COL_PATH]
        else:
            return None

    def on_partition_list_delete_activate(self, button):
        """ Delete partition """
        selection = self.partition_list.get_selection()
        if not selection:
            return

        model, tree_iter = selection.get_selected()

        if tree_iter is None:
            return

        am_new = False

        # Get row data
        row = model[tree_iter]

        mount_point = row[COL_MOUNT_POINT]
        # size_available = row[COL_SIZE]
        partition_path = row[COL_PARTITION_PATH]

        if mount_point in self.diskdic['mounts']:
            self.diskdic['mounts'].remove(mount_point)

        uid = self.gen_partition_uid(path=partition_path)

        if uid in self.stage_opts:
            am_new = self.stage_opts[uid][0]
            del self.stage_opts[uid]

        if uid in self.luks_options:
            del self.luks_options[uid]

        if not am_new:
            for orig_part in self.orig_part_dic:
                if uid == self.orig_part_dic[orig_part]:
                    self.to_be_deleted.append(orig_part)

        disk_path = self.get_disk_path_from_selection(model, tree_iter)
        self.disks_changed.append(disk_path)

        logging.info(_("You will delete the partition %s from disk %s"), partition_path, disk_path)

        # Be sure to just call get_devices once
        if self.disks is None:
            self.disks = pm.get_devices()

        (disk, result) = self.disks[disk_path]

        partitions = pm.get_partitions(disk)

        # used_dic ?
        part = partitions[partition_path]
        if (disk.device.path, part.geometry.start) in self.used_dic:
            del self.used_dic[(disk.device.path, part.geometry.start)]

        # Before delete the partition, check if it's already mounted
        if pm.check_mounted(part):
            # We unmount the partition. Should we ask first?
            logging.info(_("Unmounting %s..."), part.path)
            try:
                subprocess.Popen(['umount', part.path], stdout=subprocess.PIPE)
            except subprocess.CalledProcessError as process_error:
                logging.error(process_error)

        # Is it worth to show some warning message here?
        # No, created delete list as part of confirmation screen.
        pm.delete_partition(disk, part)

        # Update the partition list treeview
        self.update_view()

    @staticmethod
    def get_mount_point(partition_path):
        """ Get device mount point """
        fs_name = ''
        fs_type = ''
        writable = ''
        with open('/proc/mounts') as fp:
            for line in fp:
                line = line.split()
                if line[0] == partition_path:
                    fs_name = line[1]
                    fs_type = line[2]
                    writable = line[3].split(',')[0]
        return fs_name, fs_type, writable

    @staticmethod
    def get_swap_partition(partition_path):
        """ Get active swap partition """

        partition = ''
        with open('/proc/swaps') as fp:
            for line in fp:
                line = line.split()
                if line[0] == partition_path:
                    partition = line[0]
        return partition

    def on_partition_list_new_activate(self, button):
        """ Add a new partition """
        selection = self.partition_list.get_selection()

        if not selection:
            return

        model, tree_iter = selection.get_selected()
        if tree_iter is None:
            return

        # Assume it will be formatted, unless it's extended (see below)
        formatme = True

        # Get necessary row data
        row = model[tree_iter]

        # Get partition type from the user selection
        part_type = row[COL_PARTITION_TYPE]

        # Check that the user has selected a free space row.
        if part_type not in (pm.PARTITION_FREESPACE, pm.PARTITION_FREESPACE_EXTENDED):
            return

        # size_available = row[COL_SIZE]
        partition_path = row[COL_PARTITION_PATH]

        # Get our parent drive
        parent_iter = model.iter_parent(tree_iter)
        parent_part_type = model[parent_iter][COL_PARTITION_TYPE]

        if parent_part_type == pm.PARTITION_EXTENDED:
            # We're creating a partition inside an already created extended
            # partition (a logical one, though). Our drive won't be our father but our grandpa
            # (we have to skip the extended partition we're in)
            parent_iter = model.iter_parent(parent_iter)
            is_primary_or_extended = False
        else:
            is_primary_or_extended = True

        disk_path = model[parent_iter][COL_PATH]
        self.disks_changed.append(disk_path)

        # Be sure to just call get_devices once
        if self.disks is None:
            self.disks = pm.get_devices()

        (disk, result) = self.disks[disk_path]

        # Added extended, moving extended details up here

        # Get the objects from the dialog
        radio = {"primary": self.ui.get_object('create_partition_create_type_primary'),
                 "logical": self.ui.get_object('create_partition_create_type_logical'),
                 "extended": self.ui.get_object('create_partition_create_type_extended')}

        radio["primary"].set_active(True)
        radio["logical"].set_active(False)
        radio["extended"].set_active(False)

        radio["primary"].set_visible(True)
        radio["logical"].set_visible(True)
        radio["extended"].set_visible(True)

        supports_extended = disk.supportsFeature(pm.DISK_EXTENDED)

        if not supports_extended:
            radio["extended"].set_visible(False)

        extended = disk.getExtendedPartition()

        if is_primary_or_extended and extended:
            radio["logical"].set_visible(False)
            radio["extended"].set_visible(False)
        elif is_primary_or_extended and not extended:
            radio["logical"].set_visible(False)
        elif not is_primary_or_extended:
            radio["primary"].set_visible(False)
            radio["logical"].set_active(True)
            radio["extended"].set_visible(False)

        if is_primary_or_extended:
            # Get how many primary partitions are already created on disk
            primary_count = disk.primaryPartitionCount
            if primary_count == disk.maxPrimaryPartitionCount:
                msg = _("Sorry, you already have {0} primary partitions created.").format(primary_count)
                show.warning(self.get_toplevel(), msg)
                return
            elif primary_count >= (disk.maxPrimaryPartitionCount - 1) and extended:
                msg = _("Sorry, you already have {0} primary and 1 extended partitions created.").format(primary_count)
                show.warning(self.get_toplevel(), msg)
                return

        radio["begin"] = self.ui.get_object('create_partition_create_place_beginning')
        radio["end"] = self.ui.get_object('create_partition_create_place_end')

        radio["begin"].set_active(True)
        radio["end"].set_active(False)

        # Prepare size spin

        dev = disk.device
        partitions = pm.get_partitions(disk)
        partition = partitions[partition_path]

        # +1 as not to leave unusably small space behind
        max_size_mb = int((partition.geometry.length * dev.sectorSize) / 1000000) + 1

        size_spin = self.ui.get_object('create_partition_size_spinbutton')
        size_spin.set_digits(0)
        adjustment = Gtk.Adjustment(
            value=max_size_mb,
            lower=1,
            upper=max_size_mb,
            step_increment=1,
            page_increment=10,
            page_size=0)
        size_spin.set_adjustment(adjustment)
        size_spin.set_value(max_size_mb)

        # label
        label_entry = self.ui.get_object('create_partition_label_entry')
        label_entry.set_text("")

        # use as (fs)
        fs_combo = self.ui.get_object('create_partition_use_combo')
        fs_combo.set_active(3)

        # mount combo entry
        mount_combo = self.ui.get_object('create_partition_mount_combo_entry')
        mount_combo.set_text("")

        # Empty tmp luks options (for encryption properties dialog)
        self.tmp_luks_options = (False, "", "")

        # Dialog windows should be set transient for the main application window they were spawned from.
        self.create_partition_dialog.set_transient_for(self.get_toplevel())

        # Finally, show the create partition dialog
        response = self.create_partition_dialog.run()
        if response == Gtk.ResponseType.OK:
            mylabel = label_entry.get_text()
            mymount = mount_combo.get_text().strip()
            if mymount in self.diskdic['mounts']:
                show.warning(self.get_toplevel(), _("Can't use same mount point twice..."))
            else:
                if mymount:
                    self.diskdic['mounts'].append(mymount)

                myfs = fs_combo.get_active_text()

                if myfs is None:
                    myfs = ""

                if myfs == 'swap':
                    mymount = 'swap'

                # Get selected size
                size = int(size_spin.get_value())

                beg_var = radio["begin"].get_active()

                start_sector = partition.geometry.start
                end_sector = partition.geometry.end
                geometry = pm.geom_builder(disk, start_sector, end_sector, size, beg_var)

                # User wants to create an extended, logical or primary partition

                if radio["primary"].get_active():
                    logging.debug(_("Creating a primary partition"))
                    pm.create_partition(disk, pm.PARTITION_PRIMARY, geometry)
                elif radio["extended"].get_active():
                    # Not mounting extended partitions.
                    if mymount:
                        self.diskdic['mounts'].remove(mymount)
                        mymount = ''
                    # And no labeling either
                    mylabel = ''
                    myfs = 'extended'
                    formatme = False
                    logging.debug(_("Creating an extended partition"))
                    pm.create_partition(disk, pm.PARTITION_EXTENDED, geometry)
                elif radio["logical"].get_active():
                    logical_count = len(list(disk.getLogicalPartitions()))
                    max_logicals = disk.getMaxLogicalPartitions()
                    if logical_count < max_logicals:
                        logging.debug(_("Creating a logical partition"))
                        pm.create_partition(disk, pm.PARTITION_LOGICAL, geometry)

                # Store new stage partition info in self.stage_opts

                old_parts = []
                for y in self.all_partitions:
                    for z in y:
                        old_parts.append(z)
                partitions = pm.get_partitions(disk)
                for e in partitions:
                    if e not in old_parts:
                        uid = self.gen_partition_uid(partition=partitions[e])
                        self.stage_opts[uid] = (True, mylabel, mymount, myfs, formatme)
                        self.luks_options[uid] = self.tmp_luks_options
                        if mymount == "/":
                            # Set if we'll be using LUKS in the root partition (for process.py to know)
                            self.settings.set('use_luks_in_root', self.tmp_luks_options[0])
                            self.settings.set('luks_root_volume', self.tmp_luks_options[1])

                # Update partition list treeview
                self.update_view()

        self.create_partition_dialog.hide()

    def on_edit_partition_encryption_settings_clicked(self, widget):
        self.partition_encryption_settings_clicked(widget)

    def on_create_partition_encryption_settings_clicked(self, widget):
        self.partition_encryption_settings_clicked(widget)

    def partition_encryption_settings_clicked(self, widget):
        """ Show LUKS encryption options dialog """
        # TODO: Check Password confirmation (compare both entries)

        entry_vol_name = self.ui.get_object('luks_vol_name_entry')
        entry_password = self.ui.get_object('luks_password_entry')
        entry_password_confirm = self.ui.get_object('luks_password_confirm_entry')

        (use_luks, vol_name, password) = self.tmp_luks_options

        entry_vol_name.set_text(vol_name)
        entry_password.set_text(password)
        entry_password_confirm.set_text(password)

        switch_use_luks = self.ui.get_object('luks_use_luks_switch')
        switch_use_luks.set_active(use_luks)
        self.enable_luks_widgets(use_luks)

        # Dialog windows should be set transient for the main application window they were spawned from.
        self.luks_dialog.set_transient_for(self.get_toplevel())

        response = self.luks_dialog.run()
        if response == Gtk.ResponseType.OK:
            password = entry_password.get_text()
            password_confirm = entry_password_confirm.get_text()
            if password == password_confirm:
                # Save new choices
                use_luks = switch_use_luks.get_active()
                vol_name = entry_vol_name.get_text()
                self.tmp_luks_options = (use_luks, vol_name, password)
            else:
                show.warning(self.get_toplevel(), _("LUKS passwords do not match!"))

        self.luks_dialog.hide()

    def on_luks_use_luks_switch_activate(self, widget, data):
        self.enable_luks_widgets(widget.get_active())

    def enable_luks_widgets(self, status):
        """ Enables or disables the LUKS encryption dialog widgets """
        w_sensitive = ['luks_vol_name_label', 'luks_password_label',
                       'luks_password_confirm_label', 'luks_vol_name_entry',
                       'luks_password_entry', 'luks_password_confirm_entry']
        w_hide = ['luks_password_confirm_image', 'luks_password_status_label']

        for w_name in w_sensitive:
            widget = self.ui.get_object(w_name)
            widget.set_sensitive(status)

        if status is False:
            for w_name in w_hide:
                widget = self.ui.get_object(w_name)
                widget.hide()

        widget = self.ui.get_object('luks_use_luks_switch')
        widget.set_active(status)

    def on_create_partition_create_type_extended_toggled(self, widget):
        """ If user selects to create an extended partition, some widgets must be disabled """
        wdgts = {'use_label': self.ui.get_object('create_partition_use_label'),
                 'use_combo': self.ui.get_object('create_partition_use_combo'),
                 'mount_label': self.ui.get_object('create_partition_mount_label'),
                 'mount_combo': self.ui.get_object('create_partition_mount_combo'),
                 'label_label': self.ui.get_object('create_partition_label_label'),
                 'label_entry': self.ui.get_object('create_partition_label_entry')}

        sensitive = True

        if widget.get_active():
            sensitive = False

        for i in wdgts:
            wdgts[i].set_sensitive(sensitive)

    def on_create_partition_use_combo_changed(self, selection):
        """ If user selects a swap fs, it can't be mounted the usual way """
        fs_selected = selection.get_active_text()

        p_mount_combo = self.ui.get_object('create_partition_mount_combo')
        p_mount_label = self.ui.get_object('create_partition_mount_label')

        if fs_selected == 'swap':
            p_mount_combo.hide()
            p_mount_label.hide()
        else:
            p_mount_combo.show()
            p_mount_label.show()

    def on_edit_partition_use_combo_changed(self, selection):
        """ If user selects a swap fs, it can't be mounted the usual way """
        fs_selected = selection.get_active_text()

        p_mount_combo = self.ui.get_object('edit_partition_mount_combo')
        p_mount_label = self.ui.get_object('edit_partition_mount_label')

        if fs_selected == 'swap':
            p_mount_combo.hide()
            p_mount_label.hide()
        else:
            p_mount_combo.show()
            p_mount_label.show()

    def on_partition_list_undo_activate(self, button):
        """ Undo all user changes """
        # To undo user changes, we simply reload all devices
        self.disks = pm.get_devices()
        self.disks_changed = []

        # Empty stage partitions' options
        self.stage_opts = {}
        self.luks_options = {}

        # Empty to be deleted partitions list
        self.to_be_deleted = []

        # Refresh our partition treeview
        self.update_view()

    def on_partition_list_treeview_selection_changed(self, selection):
        """ Selection in treeview changed, call check_buttons to update them """
        self.check_buttons(selection)
        return False

    @staticmethod
    def on_partition_list_treeview_button_press_event(widget, event):
        """ Called when clicked on the partition list treeview.
            Not doing anything here atm (just return false to not stop the chain of events) """
        return False

    @staticmethod
    def on_partition_list_treeview_key_press_event(widget, event):
        """ Called when a key is pressed when the partition list treeview has focus.
            Not doing anything here atm (just return false to not stop the chain of events) """
        return False

    def on_partition_list_treeview_row_activated(self, path, column, user_data):
        """ Simulate a click in new or edit if a partition or free space is double clicked """
        button_edit = self.ui.get_object('partition_button_edit')
        button_new = self.ui.get_object('partition_button_new')

        if button_edit.get_sensitive():
            self.on_partition_list_edit_activate(None)
        elif button_new.get_sensitive():
            self.on_partition_list_new_activate(None)

        return False

    @staticmethod
    def on_partition_list_treeview_popup_menu(widget):
        """ Not doing anything here (return false to not stop the chain of events) """
        return False

    def translate_ui(self):
        """ As the installer language can change anytime the user changes it, we have
            to 'retranslate' all our widgets calling this function """

        self.header.set_subtitle(_("Advanced Installation Mode"))

        txt = _("Use the device below for boot loader installation:")
        txt = "<span weight='bold' size='small'>{0}</span>".format(txt)
        label = self.ui.get_object('bootloader_device_info_label')
        label.set_markup(txt)

        txt = _("Bootloader:")
        label = self.ui.get_object('bootloader_label')
        label.set_markup(txt)

        txt = _("Device:")
        label = self.ui.get_object('bootloader_device_label')
        label.set_markup(txt)

        txt = _("Mount Checklist:")
        txt = "<span weight='bold'>{0}</span>".format(txt)
        label = self.ui.get_object('mnt_chklist')
        label.set_markup(txt)

        part = self.ui.get_object('root_part')
        txt = _("Root")
        part.props.label = txt + " ( / )"

        part = self.ui.get_object('boot_part')
        txt = _("Boot")
        part.props.label = txt + " ( /boot )"

        part = self.ui.get_object('boot_efi_part')
        txt = _("EFI")
        part.props.label = txt + " ( /boot/efi fat32 )"

        part = self.ui.get_object('swap_part')
        txt = _("Swap")
        part.props.label = txt

        # txt = _("TODO: Here goes a warning message")
        # txt = "<span weight='bold'>{0}</span>".format(txt)
        # label = self.ui.get_object('part_advanced_warning_message')
        # label.set_markup(txt)

        # Assign labels to buttons
        btn = self.ui.get_object("partition_button_undo")
        btn.set_label(_("Undo"))

        btn = self.ui.get_object("partition_button_new")
        btn.set_label(_("New"))

        btn = self.ui.get_object("partition_button_delete")
        btn.set_label(_("Delete"))

        btn = self.ui.get_object("partition_button_edit")
        btn.set_label(_("Edit..."))

        btn = self.ui.get_object("partition_button_new_label")
        btn.set_label(_("New partition table"))

        btns = [
            "changelist_cancelbutton",
            "create_table_dialog_cancel",
            "edit_partition_cancel",
            "create_partition_cancel",
            "luks_cancel_button"]

        for btn_id in btns:
            btn = self.ui.get_object(btn_id)
            btn.set_label(_("_Cancel"))

        btns = [
            "changelist_okbutton",
            "create_table_dialog_ok",
            "edit_partition_ok",
            "create_partition_ok",
            "luks_ok_button"]

        for btn_id in btns:
            btn = self.ui.get_object(btn_id)
            btn.set_label(_("_Apply"))

        # Translate dialog "Create partition"
        txt = _("Size:")
        label = self.ui.get_object('create_partition_size_label')
        label.set_markup(txt)

        txt = _("Type:")
        label = self.ui.get_object('create_partition_create_type_label')
        label.set_markup(txt)

        txt = _("Primary")
        button = self.ui.get_object('create_partition_create_type_primary')
        button.set_label(txt)

        txt = _("Logical")
        button = self.ui.get_object('create_partition_create_type_logical')
        button.set_label(txt)

        txt = _("Extended")
        button = self.ui.get_object('create_partition_create_type_extended')
        button.set_label(txt)

        txt = _("Beginning of this space")
        button = self.ui.get_object('create_partition_create_place_beginning')
        button.set_label(txt)

        txt = _("End of this space")
        button = self.ui.get_object('create_partition_create_place_end')
        button.set_label(txt)

        txt = _("Use As:")
        label = self.ui.get_object('create_partition_use_label')
        label.set_markup(txt)

        txt = _("Mount Point:")
        label = self.ui.get_object('create_partition_mount_label')
        label.set_markup(txt)

        txt = _("Label (optional):")
        label = self.ui.get_object('create_partition_label_label')
        label.set_markup(txt)

        txt = _("Encryption Options...")
        button = self.ui.get_object('create_partition_encryption_settings')
        button.set_label(txt)

        # Translate dialog "Edit partition"
        txt = _("Use As:")
        label = self.ui.get_object('edit_partition_use_label')
        label.set_markup(txt)

        txt = _("Mount Point:")
        label = self.ui.get_object('edit_partition_mount_label')
        label.set_markup(txt)

        txt = _("Label (optional):")
        label = self.ui.get_object('edit_partition_label_label')
        label.set_markup(txt)

        txt = _("Format:")
        label = self.ui.get_object('edit_partition_format_label')
        label.set_markup(txt)

        txt = _("Encryption Options...")
        button = self.ui.get_object('edit_partition_encryption_settings')
        button.set_label(txt)

        # Create disk partition table dialog
        txt = _("Partition Table Type:")
        label = self.ui.get_object('partition_type_label')
        label.set_markup(txt)

        dialog = self.ui.get_object("create_table_dialog")
        dialog.set_title(_("Create Partition Table"))

        # LUKS options dialog
        txt = _("Use LUKS encryption:")
        label = self.ui.get_object('luks_use_luks_label')
        label.set_markup(txt)

        txt = _("LUKS volume name:")
        label = self.ui.get_object('luks_vol_name_label')
        label.set_markup(txt)

        txt = _("Password:")
        label = self.ui.get_object('luks_password_label')
        label.set_markup(txt)

        txt = _("Confirm password:")
        label = self.ui.get_object('luks_password_confirm_label')
        label.set_markup(txt)

    def prepare(self, direction):
        """ Prepare our dialog to show/hide/activate/deactivate what's necessary """

        self.translate_ui()
        self.update_view()
        self.show_all()

        self.fill_bootloader_entry()

        # button = self.ui.get_object('create_partition_encryption_settings')
        # button = self.ui.get_object('edit_partition_encryption_settings')

        # label = self.ui.get_object('part_advanced_recalculating_label')
        # label.hide()
        # spinner = self.ui.get_object('partition_recalculating_spinner')
        # spinner.hide()

        button = self.ui.get_object('partition_button_lvm')
        button.hide()

        widget_ids = [
            'partition_button_new', 'partition_button_delete', 'partition_button_edit',
            'partition_button_new_label', 'partition_button_undo']

        for widget_id in widget_ids:
            button = self.ui.get_object(widget_id)
            button.set_sensitive(False)

    def on_partition_list_new_label_activate(self, button):
        """ Create a new partition table """
        # TODO: We should check first if there's any mounted partition (including swap)

        selection = self.partition_list.get_selection()

        if not selection:
            return

        model, tree_iter = selection.get_selected()

        if tree_iter is None:
            return

        disk_path = model[tree_iter][0]

        # Be sure to just call get_devices once
        if self.disks is None:
            self.disks = pm.get_devices()

        # disk_sel, result = self.disks[disk_path]

        dialog = self.ui.get_object("create_table_dialog")

        # Dialog windows should be set transient for the main application window they were spawned from.
        dialog.set_transient_for(self.get_toplevel())

        response = dialog.run()
        if response == Gtk.ResponseType.OK:
            combo = self.ui.get_object('partition_types_combo')
            line = combo.get_active_text()
            if line:
                # by default, use msdos type
                ptype = 'msdos'

                if "GPT" in line:
                    ptype = 'gpt'

                logging.info(_("Creating a new %s partition table for disk %s"), ptype, disk_path)

                new_disk = pm.make_new_disk(disk_path, ptype)
                self.disks[disk_path] = (new_disk, pm.OK)

                self.update_view()

                if ptype == 'gpt' and not os.path.exists('/sys/firmware/efi'):
                    # Show warning (see https://github.com/Apricity/Cnchi/issues/63)
                    msg = _(
                        'GRUB requires a BIOS Boot Partition in BIOS systems to embed its core.img file due to lack of '
                        'post-MBR embed gap in GPT disks.\n\n'
                        'Cnchi will create this BIOS Boot Partition for you.')
                    show.warning(self.get_toplevel(), msg)
                    self.create_bios_gpt_boot_partition(disk_path)

        dialog.hide()

    def create_bios_gpt_boot_partition(self, disk_path):
        """ Create an unformatted partition with no filesystem and with a bios_grub flag on. """

        self.disks_changed.append(disk_path)

        # Be sure to just call get_devices once
        if self.disks is None:
            self.disks = pm.get_devices()

        (disk, result) = self.disks[disk_path]

        partitions = pm.get_partitions(disk)
        partition_list = pm.order_partitions(partitions)
        p = None
        for partition_path in partition_list:
            p = partitions[partition_path]

        # Get how many primary partitions are already created on disk
        if p is None or disk.primaryPartitionCount > 0:
            # BIOS GPT Boot partition must be the first one on the disk
            txt = _("Can't create BIOS GPT Boot partition!")
            logging.error(txt)
            show.error(self.get_toplevel(), txt)
            return

        # max_size_mb = int((p.geometry.length * dev.sectorSize) / 1000000) + 1

        mylabel = ""
        mymount = ""
        myfmt = "bios-gpt-boot"

        # It won't be formated
        formatme = False

        # Size must be 2MiB
        size = 2

        beg_var = True

        start_sector = p.geometry.start
        end_sector = p.geometry.end
        geometry = pm.geom_builder(disk, start_sector,
                                   end_sector, size, beg_var)

        part = pm.create_partition(disk, pm.PARTITION_PRIMARY, geometry)

        res, myerr = pm.set_flag(pm.PED_PARTITION_BIOS_GRUB, part)

        if res:
            txt = _("Couldn't create BIOS GPT Boot partition")
            logging.error(txt)
            logging.error(myerr)
            show.error(self.get_toplevel(), txt)

        # Store stage partition info in self.stage_opts
        old_parts = []
        for y in self.all_partitions:
            for z in y:
                old_parts.append(z)

        partitions = pm.get_partitions(disk)
        for e in partitions:
            if e not in old_parts:
                uid = self.gen_partition_uid(partition=partitions[e])
                self.stage_opts[uid] = (True, mylabel, mymount, myfmt, formatme)

        # Update partition list treeview
        self.update_view()

    def on_partition_list_lvm_activate(self, button):
        pass

    @staticmethod
    def need_swap():
        """ Returns if having a swap partition is advisable """
        with open("/proc/meminfo") as mem_info:
            mem_info_lines = mem_info.readlines()

        mem_total = ""
        for line in mem_info_lines:
            if "MemTotal" in line:
                mem_total = line
                break

        if len(mem_total) > 0:
            mem = float(mem_total.split()[1]) / 1024.0
            mem = int(mem)
        else:
            return True

        if mem < 4096:
            return True
        return False

    def check_mount_points(self):
        """
        Check that all necessary mount points are specified.
        At least root (/) partition must be defined and in UEFI systems
        a fat partition mounted in /boot (gummiboot) or /boot/efi (grub2) must be defined too.
        """

        check_parts = ["/", "/boot", "/boot/efi", "swap"]

        has_part = {}
        for check_part in check_parts:
            has_part[check_part] = False

        # Are we in a EFI system?
        is_uefi = os.path.exists('/sys/firmware/efi')

        # Be sure to just call get_devices once
        if self.disks is None:
            self.disks = pm.get_devices()

        # Get check part labels
        label_names = {"/" : "root_part",
            "/boot" : "boot_part",
            "/boot/efi" : "boot_efi_part",
            "swap" : "swap_part"}

        part_label = {}
        for check_part in check_parts:
            part_label[check_part] = self.ui.get_object(label_names[check_part])
            part_label[check_part].set_state(False)
            part_label[check_part].hide()

        # Root is always necessary
        part_label["/"].show()

        if is_uefi:
            if self.bootloader == "grub2":
                part_label["/boot/efi"].show()
            if self.bootloader == "gummiboot":
                part_label["/boot"].show()
        else:
            # LVM in non UEFI needs a /boot partition
            if self.lv_partitions:
                part_label["/boot"].show()

        if self.need_swap():
            # Low mem systems need a swap partition
            part_label["swap"].show()

        # Check mount points and filesystems
        for part_path in self.stage_opts:
            (is_new, lbl, mnt, fsystem, fmt) = self.stage_opts[part_path]

            if mnt == "/" and fsystem not in ["fat", "ntfs", "swap"]:
                has_part["/"] = True
                part_label["/"].show()
                part_label["/"].set_state(True)

            if mnt == "swap":
                has_part["swap"] = True
                part_label["swap"].show()
                part_label["swap"].set_state(True)

            if is_uefi:
                # /boot or /boot/efi need to be fat32 in UEFI systems
                if "fat" in fsystem:
                    if self.bootloader == "grub2" and mnt == "/boot/efi":
                        # Grub2 in UEFI
                        has_part["/boot/efi"] = True
                        part_label["/boot/efi"].show()
                        part_label["/boot/efi"].set_state(True)
                    elif self.bootloader == "gummiboot" and mnt == "/boot":
                        # Gummiboot
                        has_part["/boot"] = True
                        part_label["/boot"].show()
                        part_label["/boot"].set_state(True)
            else:
                if mnt == "/boot" and fsystem not in ["f2fs", "swap"]:
                    # /boot in non UEFI systems
                    has_part["/boot"] = True
                    part_label["/boot"].show()
                    part_label["/boot"].set_state(True)

        # In all cases a root partition must be defined
        check_ok = has_part["/"]

        if is_uefi:
            if self.bootloader == "grub2":
                # Grub2 needs a /boot/efi partition in UEFI
                check_ok = check_ok and has_part["/boot/efi"]
            elif self.bootloader == "gummiboot":
                # Gummiboot needs a /boot partition
                check_ok = check_ok and has_part["/boot"]
        else:
            if self.lv_partitions:
                # LVM in non UEFI needs a boot partition
                check_ok = check_ok and has_part["/boot"]

        self.forward_button.set_sensitive(check_ok)

        if check_ok:
            self.forward_button.set_name('fwd_btn_install_now')
            txt = _("Install now!")
            self.forward_button.set_label(txt)
            self.check_ok_once = True
        elif self.check_ok_once:
            self.forward_button.set_name('fwd_btn')
            self.forward_button.set_label('')
            self.check_ok_once = False

    def get_changes(self):
        """ Grab all changes for confirmation """
        changelist = []
        # Store values as (path, create?, label?, format?, mount_point, encrypt?)
        if self.lv_partitions:
            for e in self.lv_partitions:
                relabel = 'No'
                encrypt = 'No'
                uid = self.gen_partition_uid(path=e)
                if uid in self.stage_opts:
                    is_new, lbl, mnt, fsystem, fmt = self.stage_opts[uid]
                    if fmt:
                        fmt = 'Yes'
                    else:
                        fmt = 'No'
                    # Advanced method formats root by default
                    # https://github.com/Apricity/Cnchi/issues/8
                    if mnt == "/":
                        fmt = 'Yes'
                    if is_new:
                        if lbl != "":
                            relabel = 'Yes'
                        # Avoid extended and bios-gpt-boot partitions getting fmt flag true on new creation
                        if fsystem != "extended" and fsystem != "bios-gpt-boot":
                            fmt = 'Yes'
                        createme = 'Yes'
                    else:
                        if e in self.orig_label_dic:
                            if self.orig_label_dic[e] == lbl:
                                relabel = 'No'
                            else:
                                relabel = 'Yes'
                        createme = 'No'

                    if uid in self.luks_options:
                        (use_luks, vol_name, password) = self.luks_options[uid]
                        if use_luks:
                            encrypt = 'Yes'
                else:
                    relabel = 'No'
                    fmt = 'No'
                    createme = 'No'
                    mnt = ''
                    encrypt = 'No'

                if createme == 'Yes' or relabel == 'Yes' or fmt == 'Yes' or mnt or encrypt == 'Yes':
                    changelist.append((e, createme, relabel, fmt, mnt, encrypt))

        if self.disks:
            for disk_path in self.disks:
                (disk, result) = self.disks[disk_path]
                partitions = pm.get_partitions(disk)
                for partition_path in partitions:
                    # Init vars
                    relabel = 'No'
                    encrypt = 'No'
                    uid = self.gen_partition_uid(path=partition_path)
                    if uid in self.stage_opts:
                        if disk.device.busy:
                            # Check if there's some mounted partition
                            mounted = False
                            if pm.check_mounted(partitions[partition_path]):
                                mount_point, fs_type, writable = self.get_mount_point(partition_path)
                                # if "swap" in fs_type:
                                swap_partition = self.get_swap_partition(partition_path)
                                msg = ""
                                if swap_partition == partition_path:
                                    msg = _("{0} is mounted as swap.\nTo continue it has to be unmounted.\n"
                                            "Click Yes to unmount, or No to return\n").format(partition_path)
                                    mounted = True
                                elif len(mount_point) > 0:
                                    msg = _("{0} is mounted in '{1}'.\nTo continue it has to be unmounted.\n"
                                            "Click Yes to unmount, or No to return\n").format(partition_path,
                                                                                              mount_point)
                                    mounted = True

                                if "install" in mount_point:
                                    # If we're recovering from a failed/stopped install, there'll be
                                    # some mounted directories. Unmount them without asking.
                                    try:
                                        cmd = ['umount', '-l', partition_path]
                                        subprocess.Popen(cmd, stdout=subprocess.PIPE)
                                        logging.debug(_("%s unmounted"), mount_point)
                                    except subprocess.CalledProcessError as process_error:
                                        logging.error(process_error)
                                elif mounted:
                                    response = show.question(self.get_toplevel(), msg)
                                    if response != Gtk.ResponseType.YES:
                                        # User doesn't want to unmount, we can't go on.
                                        return []
                                    else:
                                        # unmount it!
                                        if swap_partition == partition_path:
                                            try:
                                                cmd = ['sh', '-c', 'swapoff {0}'.format(partition_path)]
                                                subprocess.Popen(cmd, stdout=subprocess.PIPE)
                                                logging.debug(_("Swap partition %s unmounted"), partition_path)
                                            except subprocess.CalledProcessError as process_error:
                                                logging.error(process_error)
                                        else:
                                            try:
                                                cmd = ['umount', partition_path]
                                                subprocess.Popen(cmd, stdout=subprocess.PIPE)
                                                logging.debug(_("%s unmounted"), mount_point)
                                            except subprocess.CalledProcessError as process_error:
                                                logging.error(process_error)
                                else:
                                    msg = _("%s shows as mounted (busy) but it has no mount point")
                                    logging.warning(msg, partition_path)

                        (is_new, lbl, mnt, fsystem, fmt) = self.stage_opts[uid]

                        if fmt:
                            fmt = 'Yes'
                        else:
                            fmt = 'No'

                        # Advanced method formats root by default
                        # https://github.com/Apricity/Cnchi/issues/8
                        if mnt == "/":
                            fmt = 'Yes'

                        if is_new:
                            if lbl != "":
                                relabel = 'Yes'
                            # Avoid extended and bios-gpt-boot partitions getting fmt flag true on new creation
                            if fsystem != "extended" and fsystem != "bios-gpt-boot":
                                fmt = 'Yes'
                            createme = 'Yes'
                        else:
                            if partition_path in self.orig_label_dic:
                                if self.orig_label_dic[partition_path] == lbl:
                                    relabel = 'No'
                                else:
                                    relabel = 'Yes'
                            createme = 'No'

                        if uid in self.luks_options:
                            (use_luks, vol_name, password) = self.luks_options[uid]
                            if use_luks:
                                encrypt = 'Yes'
                    else:
                        relabel = 'No'
                        fmt = 'No'
                        createme = 'No'
                        mnt = ''

                    if createme == 'Yes' or relabel == 'Yes' or fmt == 'Yes' or mnt or encrypt == 'Yes':
                        changelist.append((partition_path, createme, relabel, fmt, mnt, encrypt))
                        msg = _("Added %s to changelist: createme[%s] relabel[%s] fmt[%s] mnt[%s] encrypt[%s]")
                        logging.debug(msg, partition_path, createme, relabel, fmt, mnt, encrypt)

            return changelist

    def show_changes(self, changelist):
        """ Show all changes to the user before doing anything, just in case. """
        if self.show_changes_grid is not None:
            self.show_changes_grid.destroy()

        vbox = self.ui.get_object("dialog-vbox6")
        grid = Gtk.Grid()
        vbox.pack_start(grid, True, True, 2)
        self.show_changes_grid = grid

        margin = 8

        bold = "<b>{0}</b>"
        y = 0

        self.to_be_deleted.sort()

        # First, show partitions that will be deleted
        for ea in self.to_be_deleted:
            lbl_text = _("Partition {0} will be deleted").format(ea)
            lbl = Gtk.Label(label=lbl_text, margin=margin)
            lbl.set_alignment(0, 0.5)
            grid.attach(lbl, 0, y, 4, 1)
            y += 1

        if changelist:
            # Partitions that will be modified (header)
            labels = [_("Partition"), _("New"), _("Relabel"), _("Format"), _("Mount"), _("Encrypt")]

            x = 0
            for txt in labels:
                lbl = Gtk.Label(margin=margin)
                lbl.set_markup(bold.format(txt))
                grid.attach(lbl, x, y, 1, 1)
                x += 1

            y += 1

            # Partitions that will be modified
            for ea in changelist:
                x = 0
                for txt in ea:
                    lbl = Gtk.Label(label=txt, margin=margin)
                    grid.attach(lbl, x, y, 1, 1)
                    x += 1
                y += 1

        changelist_dialog = self.ui.get_object("changelist_dialog")
        changelist_dialog.set_title(_('These disks will have partition actions:'))
        changelist_dialog.show_all()

        # Dialog windows should be set transient for the main application window they were spawned from.
        changelist_dialog.set_transient_for(self.get_toplevel())

        response = changelist_dialog.run()
        changelist_dialog.hide()
        while Gtk.events_pending():
            Gtk.main_iteration()

        return response

    @staticmethod
    def set_cursor(cursor_type):
        """ Sets mouse cursor in root window """
        gdk_screen = Gdk.Screen.get_default()
        if gdk_screen is not None:
            gdk_window = gdk_screen.get_root_window()
            if gdk_window is not None:
                gdk_window.set_cursor(Gdk.Cursor(cursor_type))

    def store_values(self):
        """ The user clicks 'Install now!' """
        changelist = self.get_changes()
        if not changelist:
            # Something wrong has happened or there's nothing to change
            return False

        changelist.sort()

        self.disable_all_widgets()

        response = self.show_changes(changelist)
        if response == Gtk.ResponseType.CANCEL:
            self.enable_all_widgets()
            return False

        self.set_cursor(Gdk.CursorType.WATCH)
        # self.stop_advanced_progressbar = False
        # self.advanced_progressbar_timeout_id = GLib.timeout_add(1000, self.on_advanced_progressbar_timeout)

        # Apply partition changes
        self.create_staged_partitions()

        # Start the installation process
        self.start_installation()

        self.set_cursor(Gdk.CursorType.LEFT_PTR)
        # self.stop_advanced_progressbar = True
        self.enable_all_widgets()

        return True

    def disable_all_widgets(self):
        self.enable_all_widgets(status=False)

    def enable_all_widgets(self, status=True):
        widgets = ["partition_list_scrolledwindow", "partition_list_treeview", "box2", "box3", "box4"]
        for name in widgets:
            widget = self.ui.get_object(name)
            widget.set_sensitive(status)
        while Gtk.events_pending():
            Gtk.main_iteration()

    '''
    def on_advanced_progressbar_timeout(self):
        """ Update value on the progress bar """
        if self.stop_advanced_progressbar:
            return False
        else:
            self.advanced_progressbar.pulse()
            while Gtk.events_pending():
                Gtk.main_iteration()
            return True
    '''

    def create_staged_partitions(self):
        """ Create staged partitions """
        # Sometimes a swap partition can still be active at this point
        try:
            cmd = ["swapon", "--show=NAME", "--noheadings"]
            swaps = subprocess.check_output(cmd).decode().split("\n")
            for name in filter(None, swaps):
                if "/dev/zram" not in name:
                    cmd = ['sh', '-c', 'swapoff {0}'.format(name)]
                    subprocess.Popen(cmd, stdout=subprocess.PIPE)
        except subprocess.CalledProcessError as process_error:
            logging.error(process_error)

        # We'll use auto_partition.setup_luks if necessary
        from installation import auto_partition as ap

        partitions = {}
        if self.disks is not None:
            for disk_path in self.disks:
                (disk, result) = self.disks[disk_path]
                # Only commit changes to disks we've changed!
                if disk_path in self.disks_changed:
                    pm.finalize_changes(disk)
                    logging.info(_("Finished saving changes in %s"), disk_path)
                # Now that partitions are created, set fs and label
                partitions.update(pm.get_partitions(disk))

            apartitions = list(partitions) + self.lv_partitions

            # Checks if a boot partition exists
            noboot = True
            for allopts in self.stage_opts:
                if self.stage_opts[allopts][2] == '/boot' or self.stage_opts[allopts][2] == '/boot/efi':
                    noboot = False

            for partition_path in apartitions:
                # Get label, mount point and filesystem of staged partitions
                uid = self.gen_partition_uid(path=partition_path)
                if uid in self.stage_opts:
                    is_new, lbl, mnt, fisy, fmt = self.stage_opts[uid]
                    # Do not label or format extended or bios-gpt-boot partitions
                    if fisy == "extended" or fisy == "bios-gpt-boot":
                        continue
                    if len(lbl) > 0:
                        txt = _("Creating new {0} filesystem in {1} labeled {2}")
                        txt = txt.format(fisy, partition_path, lbl)
                    else:
                        txt = _("Creating new {0} filesystem in {1}")
                        txt = txt.format(fisy, partition_path)

                    logging.info(txt)

                    if ((mnt == '/' and noboot) or mnt == '/boot') and '/dev/mapper' not in partition_path:
                        if not pm.get_flag(partitions[partition_path], pm.PED_PARTITION_BOOT):
                            if self.bootloader_device or self.bootloader_device is not None:
                                pm.set_flag(pm.PED_PARTITION_BOOT, partitions[partition_path])
                        if not self.testing:
                            pm.finalize_changes(partitions[partition_path].disk)
                    if "/dev/mapper" in partition_path:
                        pvs = lvm.get_lvm_partitions()
                        vgname = partition_path.split("/")[-1]
                        vgname = vgname.split('-')[0]
                        if mnt == '/' or mnt == '/boot':
                            self.blvm = True
                            if noboot or mnt == '/boot':
                                for ee in pvs[vgname]:
                                    # print(partitions)
                                    if not pm.get_flag(partitions[ee], pm.PED_PARTITION_BOOT):
                                        if self.bootloader_device or self.bootloader_device is not None:
                                            pm.set_flag(pm.PED_PARTITION_BOOT, partitions[ee])
                                    if not self.testing:
                                        pm.finalize_changes(partitions[ee].disk)

                    if uid in self.luks_options:
                        (use_luks, vol_name, password) = self.luks_options[uid]
                        if use_luks and len(vol_name) > 0 and len(password) > 0:
                            txt = _("Encrypting {0}, assigning volume name {1} and formatting it...")
                            txt = txt.format(partition_path, vol_name)
                            logging.info(txt)
                            if not self.testing:
                                # Do real encryption here!
                                # TODO: Show a progress dialog here as setup_luks is slow
                                with misc.raised_privileges():
                                    ap.setup_luks(luks_device=partition_path, luks_name=vol_name,
                                                  luks_pass=password)
                                self.settings.set("use_luks", True)
                                luks_device = "/dev/mapper/" + vol_name
                                error, msg = fs.create_fs(luks_device, fisy, lbl)
                                if not error:
                                    logging.info(msg)
                                else:
                                    txt = _("Couldn't format LUKS device '{0}' with label '{1}' as '{2}'")
                                    txt = txt.format(luks_device, lbl, fisy)
                                    logging.error(txt)
                                    logging.error(msg)
                                    show.error(self.get_toplevel(), txt)

                                # Do not format (already done)
                                fmt = False
                                # Do not relabel (already done)
                                if partition_path in self.orig_label_dic:
                                    lbl = self.orig_label_dic[partition_path]
                                if mnt == "/":
                                    self.settings.set("luks_root_password", password)
                                    self.settings.set("luks_root_device", partition_path)

                    # Only format if they want formatting
                    if fmt:
                        # All of fs module takes paths, not partition objs
                        if not self.testing:
                            # Create filesystem using mkfs
                            error, msg = fs.create_fs(partition_path, fisy, lbl)
                            if not error:
                                logging.info(msg)
                            else:
                                txt = _("Couldn't format partition '{0}' with label '{1}' as '{2}'")
                                txt = txt.format(partition_path, lbl, fisy)
                                logging.error(txt)
                                logging.error(msg)
                                show.error(self.get_toplevel(), txt)
                    elif partition_path in self.orig_label_dic:
                        if self.orig_label_dic[partition_path] != lbl:
                            if not self.testing:
                                try:
                                    fs.label_fs(fisy, partition_path, lbl)
                                except Exception as label_error:
                                    # Catch all exceptions because not being able to label
                                    # a partition shouldn't be fatal
                                    logging.error(label_error)

    def start_installation(self):
        """ Start installation process """

        # Fill fs_devices and mount_devices dicts that are going to be used by InstallationProcess
        fs_devices = {}
        mount_devices = {}
        for disk_path in self.disks:
            (disk, result) = self.disks[disk_path]
            partitions = pm.get_partitions(disk)
            self.all_partitions.append(partitions)
            partition_list = pm.order_partitions(partitions)
            fs_type = ""
            for ppath in self.lv_partitions:
                uid = self.gen_partition_uid(path=ppath)
                if uid in self.stage_opts:
                    (is_new, label, mount_point, fs_type, fmt_active) = self.stage_opts[uid]
                    mount_devices[mount_point] = ppath
                    fs_devices[ppath] = fs_type
                if uid in self.luks_options:
                    (use_luks, vol_name, password) = self.luks_options[uid]
                    if use_luks and len(vol_name) > 0:
                        mount_devices[mount_point] = "/dev/mapper/" + vol_name
            for partition_path in partition_list:
                uid = self.gen_partition_uid(partition=partitions[partition_path])
                if uid in self.stage_opts:
                    (is_new, label, mount_point, fs_type, fmt_active) = self.stage_opts[uid]
                    if fs_type == "extended" or fs_type == "bios-gpt-boot":
                        # Do not mount extended or bios-gpt-boot partitions
                        continue
                    mount_devices[mount_point] = partition_path
                    fs_devices[partition_path] = fs_type

                if uid in self.luks_options:
                    (use_luks, vol_name, password) = self.luks_options[uid]
                    if use_luks and len(vol_name) > 0:
                        luks_device = "/dev/mapper/" + vol_name
                        mount_devices[mount_point] = luks_device
                        del fs_devices[partition_path]
                        fs_devices[luks_device] = fs_type

        checkbox = self.ui.get_object("bootloader_device_check")
        if checkbox.get_active() is False:
            self.settings.set('bootloader_install', False)
            logging.warning(_("Cnchi will not install any bootloader"))
        else:
            self.settings.set('bootloader_install', True)
            self.settings.set('bootloader_device', self.bootloader_device)

            self.settings.set('bootloader', self.bootloader)
            msg = _("Apricity OS will install the bootloader {0} in device {1}")
            msg = msg.format(self.bootloader, self.bootloader_device)
            logging.info(msg)

        if not self.testing:
            self.process = installation_process.InstallationProcess(
                self.settings,
                self.callback_queue,
                mount_devices,
                fs_devices,
                self.alternate_package_list,
                self.ssd,
                self.blvm)

            self.process.start()
        else:
            logging.warning(_("Testing mode. Cnchi will not change anything!"))

# When testing, no _() is available
try:
    _("")
except NameError as err:
    def _(message):
        return message

if __name__ == '__main__':
    from test_screen import _, run

    run('InstallationAdvanced')

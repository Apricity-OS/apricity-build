/*
 * Copyright 2011 Axel von Bertoldi
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to:
 * The Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1301, USA.
 */

const St = imports.gi.St;
const Main = imports.ui.main;
const PopupMenu = imports.ui.popupMenu;
const PanelMenu = imports.ui.panelMenu;
const ModalDialog = imports.ui.modalDialog;
const Gio = imports.gi.Gio;
const Gtk = imports.gi.Gtk;
const Lang = imports.lang;
const Clutter = imports.gi.Clutter;

const Gettext = imports.gettext.domain("gnome-shell-trash-extension");
const _ = Gettext.gettext;


const TrashMenuItem = new Lang.Class({
    Name: 'TrashMenuItem.TrashMenuItem',
    Extends: PopupMenu.PopupBaseMenuItem,

    _init: function(text, icon_name, gicon, callback) {
        this.parent(0.0, text);

        let icon_cfg = { style_class: 'popup-menu-icon' };
        if (icon_name != null) {
          icon_cfg.icon_name = icon_name;
        } else if (gicon != null) {
          icon_cfg.gicon = gicon;
        }

        this.icon = new St.Icon(icon_cfg);
        this.actor.add_child(this.icon);
        this.label = new St.Label({ text: text });
        this.actor.add_child(this.label);

        this.connect('activate', callback);
    },

    destroy: function() {
        this.parent();
    }
});


const TrashMenu = new Lang.Class({
    Name: 'TrashMenu.TrashMenu',
    Extends: PanelMenu.Button,

    _init: function() {
        this.parent(0.0, _("Trash"));
        this.trashIcon = new St.Icon({ icon_name: 'user-trash-full-symbolic',
                                       style_class: 'popup-menu-icon' })
      	this.actor.add_actor(this.trashIcon);

        this.trash_path = 'trash:///';
        this.trash_file = Gio.file_new_for_uri(this.trash_path);

        this._addConstMenuItems();
        this._onTrashChange();
        this._setupWatch();
    },

    _addConstMenuItems: function() {
        this.empty_item = new TrashMenuItem(_("Empty Trash"),
                                            "edit-delete-symbolic",
                                            null,
                                            Lang.bind(this, this._onEmptyTrash));
        this.menu.addMenuItem(this.empty_item);

        this.open_item = new TrashMenuItem(_("Open Trash"),
                                           "folder-open-symbolic",
                                           null,
                                           Lang.bind(this, this._onOpenTrash));
        this.menu.addMenuItem(this.open_item);

        this.separator = new PopupMenu.PopupSeparatorMenuItem();
        this.menu.addMenuItem(this.separator);
    },

    destroy: function() {
        this.parent();
    },

    _onOpenTrash: function() {
        Gio.app_info_launch_default_for_uri(this.trash_path, null);
    },

    _setupWatch: function() {
        this.monitor = this.trash_file.monitor_directory(0, null, null);
        this.monitor.connect('changed', Lang.bind(this, this._onTrashChange));
    },

    _onTrashChange: function() {
      this._clearMenu();
      if (this._listFilesInTrash() == 0) {
          this.actor.visible = false;
      } else {
          this.actor.show();
          this.actor.visible = true;
      }
    },

    _onEmptyTrash: function() {
      new ConfirmEmptyTrashDialog(Lang.bind(this, this._doEmptyTrash)).open();
    },

    _doEmptyTrash: function() {
      let children = this.trash_file.enumerate_children('*', 0, null, null);
      let child_info = null;
      while ((child_info = children.next_file(null, null)) != null) {
        let child = this.trash_file.get_child(child_info.get_name());
        child.delete(null);
      }
    },

    _listFilesInTrash: function() {
      let children = this.trash_file.enumerate_children('*', 0, null, null);
      let count  = 0;
      let file_info = null;
      while ((file_info = children.next_file(null, null)) != null) {
        let file_name  = file_info.get_name();
        let item = new TrashMenuItem(file_info.get_display_name(),
                                     null,
                                     file_info.get_symbolic_icon(),
                                     Lang.bind(this, function() {
                                       this._openTrashItem(file_name);
                                     }));
        this.menu.addMenuItem(item);
        count++;
      }
      children.close(null, null)
      return count;
    },

    _clearMenu: function() {
      let existing = this.menu._getMenuItems();
      let i = existing.length - 1;
      while(i > 2) {
        existing[i].destroy();
        i--;
      }
    },

    _openTrashItem: function(file_name) {
      file = this.trash_file.get_child(file_name);
      Gio.app_info_launch_default_for_uri(file.get_uri(), null);
    }
});

const MESSAGE = _("Are you sure you want to delete all items from the trash?\n\
This operation cannot be undone.");

function ConfirmEmptyTrashDialog(emptyMethod) {
  this._init(emptyMethod);
}

ConfirmEmptyTrashDialog.prototype = {
  __proto__: ModalDialog.ModalDialog.prototype,

  _init: function(emptyMethod) {
    ModalDialog.ModalDialog.prototype._init.call(this, { styleClass: null });

    let mainContentBox = new St.BoxLayout({ style_class: 'polkit-dialog-main-layout',
                                            vertical: false });
    this.contentLayout.add(mainContentBox, { x_fill: true, y_fill: true });

    let messageBox = new St.BoxLayout({ style_class: 'polkit-dialog-message-layout',
                                        vertical: true });
    mainContentBox.add(messageBox, { y_align: St.Align.START });

    this._subjectLabel = new St.Label({ style_class: 'polkit-dialog-headline',
                                        text: _("Empty Trash?") });

    messageBox.add(this._subjectLabel, { y_fill:  false, y_align: St.Align.START });

    this._descriptionLabel = new St.Label({ style_class: 'polkit-dialog-description',
                                            text: Gettext.gettext(MESSAGE) });

    messageBox.add(this._descriptionLabel, { y_fill:  true, y_align: St.Align.START });

    this.setButtons([
      {
        label: _("Cancel"),
        action: Lang.bind(this, function() {
          this.close();
        }),
        key: Clutter.Escape
      },
      {
        label: _("Empty"),
        action: Lang.bind(this, function() {
          this.close();
          emptyMethod();
        })
      }
    ]);
  }
};

function init(extensionMeta) {
    imports.gettext.bindtextdomain("gnome-shell-trash-extension", extensionMeta.path + "/locale");
}

let _indicator;

function enable() {
  _indicator = new TrashMenu;
  Main.panel.addToStatusArea('trash_button', _indicator);
}

function disable() {
  _indicator.destroy();
}

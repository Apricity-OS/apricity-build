//  GNOME Shell Extension TaskBar
//  Copyright (C) 2015 zpydr
//
//  Version 43
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
//  zpydr@openmailbox.org

const Gio = imports.gi.Gio;

const Extension = imports.misc.extensionUtils.getCurrentExtension();

function Settings(schema)
{
    this.init(schema);
}

Settings.prototype =
{
    schema: null,

    init: function(schema)
    {
        this.schema = schema;
    },

    getSettings: function()
    {
        const GioSSS = Gio.SettingsSchemaSource;
        let schemaDir = Extension.dir.get_child('schemas');
        let schemaSource;
        if (schemaDir.query_exists(null))
            schemaSource = GioSSS.new_from_directory(schemaDir.get_path(), GioSSS.get_default(), false);
        else
            schemaSource = GioSSS.get_default();
        let schemaObj = schemaSource.lookup(this.schema, true);
        if (! schemaObj)
            throw new Error('Schema ' + this.schema + ' could not be found for extension ' + Extension.metadata.uuid + '. Please check your installation.');
        return new Gio.Settings({settings_schema: schemaObj})
    }
}

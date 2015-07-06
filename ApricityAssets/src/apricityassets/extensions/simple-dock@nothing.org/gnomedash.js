// -*- mode: js; js-indent-level: 4; indent-tabs-mode: nil -*-
/*jshint esnext: true */
/*jshint indent: 4 */
const Lang = imports.lang;
const Main = imports.ui.main;

const GnomeDash = new Lang.Class({
    Name: 'GnomeDash',

    _init: function() {
        this._dash = Main.overview._dash.actor.get_parent();
    },

    hideDash: function() {
        this._dash.hide();
    },

    showDash: function() {
        this._dash.show();
    }
});

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

const Lang = imports.lang;

function Windows(callBackThis, callbackWindowsListChanged, callbackWindowChanged)
{
    this.init(callBackThis, callbackWindowsListChanged, callbackWindowChanged);
}

Windows.prototype =
{
    workspace: null,
    windowsList: new Array(),
    callBackThis: null,
    callbackWindowsListChanged: null,
    callbackWindowChanged: null,
    workspaceSwitchSignal: null,
    windowAddedSignal: null,
    windowRemovedSignal: null,
    windowsSignals: new Array(),

    init: function(callBackThis, callbackWindowsListChanged, callbackWindowChanged)
    {
        //Set User Callback
        this.callBackThis = callBackThis;
        this.callbackWindowsListChanged = callbackWindowsListChanged;
        this.callbackWindowChanged = callbackWindowChanged;

        //Init WindowsList
        this.onWorkspaceChanged();

        //Add window manager signals
        this.workspaceSwitchSignal = global.screen.connect('workspace-switched', Lang.bind(this, this.onWorkspaceChanged));
    },

    destruct: function()
    {
        //Remove window manager signals
        global.screen.disconnect(this.workspaceSwitchSignal);

        //Remove workspace signals
        this.workspace.disconnect(this.windowAddedSignal);
        this.workspace.disconnect(this.windowRemovedSignal);

        //Clean windows list
        this.cleanWindowsList();
    },

    onWorkspaceChanged: function()
    {
        //Remove workspace signals if necessary
        if (this.windowAddedSignal !== null)
            this.workspace.disconnect(this.windowAddedSignal);
        if (this.windowRemovedSignal !== null)
            this.workspace.disconnect(this.windowRemovedSignal);

        //Clean windows list
        this.cleanWindowsList();

        //Get current workspace
        this.workspace = global.screen.get_active_workspace();

        //Build windows list
        activeWorkspace.list_windows().sort(this.sortWindowsCompareFunction).forEach(
            function(window)
            {
                this.addWindowInList(window);
            },
            this
        );

        //Call User Callback
        this.callbackWindowsListChanged.call(this.callBackThis, this.windowsList, 0, null);

        //Add workspace signals
        this.windowAddedSignal = this.workspace.connect_after('window-added', Lang.bind(this, this.onWindowAdded));
        this.windowRemovedSignal = this.workspace.connect_after('window-removed', Lang.bind(this, this.onWindowRemoved));
    },

    sortWindowsCompareFunction: function(windowA, windowB)
    {
        return windowA.get_stable_sequence() > windowB.get_stable_sequence();
    },

    onWindowChanged: function(window, object, type)
    {
        if (type === 0) //Focus changed
        {
            if (window.appears_focused)
                this.callbackWindowChanged.call(this.callBackThis, window, 0);
        }
        else if (type === 1) //Title changed
            this.callbackWindowChanged.call(this.callBackThis, window, 1);
        else if (type === 2) //Minimized
            this.callbackWindowChanged.call(this.callBackThis, window, 2);
    },

    onWindowAdded: function(workspace, window)
    {
        if (this.addWindowInList(window))
            this.callbackWindowsListChanged.call(this.callBackThis, this.windowsList, 1, window);
    },

    onWindowRemoved: function(workspace, window)
    {
        if (this.removeWindowInList(window))
            this.callbackWindowsListChanged.call(this.callBackThis, this.windowsList, 2, window);
    },

    searchWindowInList: function(window)
    {
        let index = null;
        for (let indexWindow in this.windowsList)
        {
            if (this.windowsList[indexWindow] === window)
            {
                index = indexWindow;
                break;
            }
        }
        return index;
    },

    addWindowInList: function(window)
    {
        let index = this.searchWindowInList(window);
        if (index === null && ! window.is_skip_taskbar())
        {
            this.windowsList.push(window);

            //Add window signals
            let objectAndSignals = [
                window, [
                    window.connect('notify::appears-focused', Lang.bind(this, this.onWindowChanged, 0)),
                    window.connect('notify::title', Lang.bind(this, this.onWindowChanged, 1)),
                    window.connect('notify::minimized', Lang.bind(this, this.onWindowChanged, 2))
                ]
            ];
            this.windowsSignals.push(objectAndSignals);
            return true;
        }
        else
            return false;
    },

    removeWindowInList: function(window)
    {
        let index = this.searchWindowInList(window);
        if (index !== null)
        {
            this.windowsList.splice(index, 1);

            //Remove window signals
            for (let indexSignal in this.windowsSignals)
            {
                let [object, signals] = this.windowsSignals[indexSignal];
                if (object === window)
                {
                    signals.forEach(
                        function(signal)
                        {
                            object.disconnect(signal);
                        },
                        this
                    );
                    this.windowsSignals.splice(indexSignal, 1);
                    break;
                }
            }
            return true;
        }
        else
            return false;
    },

    cleanWindowsList: function()
    {
        for (let i = this.windowsList.length -1 ; i>=0 ; i--)
            this.removeWindowInList(this.windowsList[i]);
    }
}

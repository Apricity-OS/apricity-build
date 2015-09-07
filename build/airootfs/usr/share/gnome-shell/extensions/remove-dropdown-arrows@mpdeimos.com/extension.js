/* jshint esnext:true */

const Main = imports.ui.main;
const Lang = imports.lang;
const ExtensionSystem = imports.ui.extensionSystem;

let _extensionChangedId;

function init()
{
	/* no initialization required */
}

function enable()
{
    edit(true);
    _extensionChangedId = ExtensionSystem.connect('extension-state-changed', onExtensionStateChanged);
}

function disable()
{
	ExtensionSystem.disconnect(_extensionChangedId);
	edit(false);
}

function onExtensionStateChanged()
{
    edit(true);
}

function edit(hide)
{
	for (let id in Main.panel.statusArea)
	{
	    let item = Main.panel.statusArea[id];
	    if (typeof item.actor !== 'undefined')
	    {
            recursiveEdit(item.actor, hide);
	    }  
	}
}

function recursiveEdit(widget, hide)
{
    if (widget.text === '\u25BE' || widget.text === '\u25B4' || // regular text drop down arrow (3.10)
       (widget.has_style_class_name && widget.has_style_class_name('popup-menu-arrow'))) // image drop down arrow (3.12)
    {
        if (hide)
        {
            widget.hide();
        }
        else
        {   
            widget.show();
        }
        
        return;
    }
    
    if (typeof widget.get_children !== 'undefined')
    {
        widget.get_children().forEach(function(child) { recursiveEdit(child, hide); });
    }
}

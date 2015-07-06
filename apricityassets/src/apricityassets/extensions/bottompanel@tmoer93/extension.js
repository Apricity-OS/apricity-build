
const Main = imports.ui.main;

let bottom=false;
let monitor,pB, button;

function _toTop() {
    pB.set_anchor_point(0,0);
    bottom=false;
}

function _toBottom() {
    pB.set_anchor_point(0,(-1)*(monitor.height-pB.get_height()));
    bottom=true;
}


function init() {
    monitor = Main.layoutManager.primaryMonitor;
    pB=Main.layoutManager.panelBox;
}

function enable() {
    _toBottom();
}

function disable() {
    _toTop();
}

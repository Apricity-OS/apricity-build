'use strict';

if (!self.port && !window.chrome && !window.safari) {
    throw new Error('Shouldn\'t be here');
}

var textMappings = {
    'sms-warning': 'sms_warning'
};

window.init = function() {
    Object.keys(textMappings).forEach(function(key) {
        document.getElementById(key).textContent = text.get(textMappings[key]); 
    });

    if (window.chrome) {
        chrome.runtime.sendMessage({ 'type': 'quickreply_get_mirror' }, function(mirror) {
            show(mirror);
        });
    } else if (window.safari) {
        var params = utils.getParams(location.search);
        window.resizeTo(params.width, params.height);

        var listener = function(e) {
            if (e.name == 'mirror') {
                show(e.message);
                safari.self.removeEventListener('message', listener, false);
            }
        };

        safari.self.addEventListener('message', listener, false);
        safari.self.tab.dispatchMessage('quickreply_get_mirror');
    } else {
        self.port.emit('quickreply_get_mirror');
        self.port.once('mirror', function(mirror) {
            show(mirror);
        });
    }
};

var show = function(mirror) {
    pb.track({
        'name': 'messaging_quickreply_shown',
        'package_name': mirror.package_name
    });

    document.getElementById('image').src = 'data:image/png;base64,' + mirror.icon;
    document.getElementById('title').textContent = mirror.title;
    document.getElementById('desc').textContent = 'Via ' + (mirror.package_name == 'com.pushbullet.android' ? 'SMS' : mirror.application_name);

    utils.linkify(mirror.body, document.getElementById('message'));

    var reply = document.getElementById('reply');
    var smsLengthHolder = document.getElementById('sms-length');
    var smsWarning = document.getElementById('sms-warning');

    reply.addEventListener('input', function() {
        var count = utils.smsLength(reply.value);
        smsLengthHolder.textContent = count + "/140";
        
        if (count > 140) {
            smsWarning.style.display = 'inline';
        } else {
            smsWarning.style.display = 'none';
        }
    }, false);

    reply.onkeydown = function(e) {
        if (e.keyCode == utils.ENTER && !e.shiftKey) {
            if (reply.value.length > 0) {
                sendReply(mirror, reply.value);
            }
            return false;
        }
    };

    if (mirror.package_name != 'com.pushbullet.android') {
        document.getElementById('sms-counter').style.display = 'none';
    }

    document.getElementById('sms-warning-link').onclick = function() {
        pb.openTab('https://help.pushbullet.com/articles/why-are-some-text-messages-not-sending/');
    };

    var heightNeeded = document.body.offsetHeight - window.innerHeight;
    window.resizeBy(0, heightNeeded);
};

var sendReply = function(mirror, reply) {
    pb.sendReply(mirror, reply);

    pb.track({
        'name': 'messaging_quickreply_sent',
        'package_name': mirror.package_name
    });

    setTimeout(function() {
        window.close();
    }, 120);
};

window.onunload = function() {
    try {
        localStorage['quickReplyScreenX'] = window.screenX;
        localStorage['quickReplyScreenY'] = window.screenY;
    } catch (e) {
    }
};

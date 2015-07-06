'use strict';

var pendingMirrors = {};

pb.addEventListener('signed_in', function(e) {
    pb.addEventListener('stream_message', function(e) {
        var message = e.detail;
        if (message.type != 'push' || !message.push) {
            return;
        }

        var push = message.push;
        if (push.source_device_iden&& push.source_device_iden == pb.local.device.iden) {
            return;
        } else if (push.target_device_iden && push.target_device_iden != pb.local.device.iden) {
            return;
        }

        if (push.type == 'mirror') {
            mirrorReceived(push);
        } else if (push.type == 'dismissal') {
            dismissalReceived(push);
        }
    });
});

var mirrorReceived = function(mirror) {
    if (pb.browserState == 'locked') {
        pb.log('Computer locked, not showing mirror');
        return;
    } else if (!pb.settings.showMirrors) {
        pb.log('Not showing mirror, disabled in settings');
        return;
    }

    showMirror(mirror);
};

var showMirror = function(mirror) {
    pb.log('Mirroring notification for:');
    pb.log(mirror);

    var options = { };
    options.type = 'basic';
    options.key = notificationKey(mirror);
    options.iconUrl = 'data:image/png;base64,' + mirror.icon;
    options.title = (mirror.package_name == 'com.pushbullet.android' ? '' : mirror.application_name + ': ') + (mirror.title || '');
    options.message = mirror.body || '';
    options.buttons = [];

    // Merge missed messages together, not for notifications from FB Messenger or Telegram though (they do it themselves)
    if (mirror.conversation_iden && ['org.telegram.messenger', 'com.facebook.orca'].indexOf(mirror.package_name) == -1) {
        var existing = pb.notifier.active[options.key];
        if (existing) {
            options.message = existing.message + '\n' + options.message;
            mirror.body = options.message;
        }
    }

    var sourceDevice = pb.local.devices[mirror.source_device_iden];
    if (sourceDevice) {
        options.contextMessage = String.format(text.get('mirror_context_message'),
                                               sourceDevice.nickname || sourceDevice.model,
                                               new Date().toLocaleTimeString().replace(/:\d+ /, ' '));
    }

    options.onclick = function() {
        var url = getWebUrl(mirror.package_name);
        if (window.chrome && !pb.isOpera && url) {
            pb.openTab(url);
        } else {
            if (mirror.conversation_iden) {
                openQuickReply(mirror);
            } else if (url) {
                pb.openTab(url);
            }
        }

        dismissRemote(mirror);
    };

    if (!mirror.conversation_iden && mirror.package_name != 'com.pushbullet.android') {
        options.buttons.push(muteButton(mirror));   
    }

    if (mirror.actions) {
        mirror.actions.forEach(function(action) {
            options.buttons.push({
                'title': 'Android: ' + action.label,
                'short_title': action.label,
                'iconUrl': 'action_android.png',
                'onclick': function() {
                    dismissRemote(mirror, action.trigger_key);
                }
            });
        });
    }

    if (mirror.conversation_iden || mirror.package_name == 'com.google.android.talk') {
        options.buttons.push({
            'title': text.get('reply'),
            'iconUrl': 'action_sms.png',
            'onclick': function() {
                if (mirror.conversation_iden) {
                    openQuickReply(mirror);
                } else if (mirror.package_name == 'com.google.android.talk') {
                    pb.openTab('https://help.pushbullet.com/articles/why-cant-i-reply-to-hangouts-messages/');
                }

                dismissRemote(mirror);
            }
        });
    }

    options.buttons.push({
        'title': text.get('dismiss'),
        'iconUrl': 'action_cancel.png',
        'onclick': function() {
            dismissRemote(mirror);
        }
    });

    getAndroidClickMapping();

    pendingMirrors[options.key] = options;
    utils.checkNativeClient(function(response) {
        if (!response && pendingMirrors[options.key]) {
            pb.notifier.show(options);
        }

        delete pendingMirrors[options.key];
    });
};

var muteButton = function(mirror) {
    var mute = function() {
        pb.post(pb.api + '/v2/ephemerals', {
            'type': 'push',
            'push': {
                'type': 'mute',
                'package_name': mirror.package_name,
                'source_user_iden': mirror.source_user_iden
            }
        }, function(response) {
            if (response) {
                pb.log('Muted ' + mirror.package_name);
                showUndo();
            } else {
                pb.log('Failed to mute ' + mirror.package_name);
            }
        });
    };

    var showUndo = function() {
        var undo = {
            'type': 'basic',
            'key': notificationKey(mirror),
            'title': String.format(text.get('muted_app'), mirror.application_name),
            'message': '',
            'iconUrl': 'data:image/png;base64,' + mirror.icon,
            'priority': 0
        };

        undo.buttons = [{
            'title': String.format(text.get('unmute_app'), mirror.application_name),
            'iconUrl': 'action_undo.png',
            'onclick': function() {
                pb.post(pb.api + '/v2/ephemerals', {
                    'type': 'push',
                    'push': {
                        'type': 'unmute',
                        'package_name': mirror.package_name,
                        'source_user_iden': mirror.source_user_iden
                    }
                }, function(response) {
                    if (response) {
                        pb.log('Unmuted ' + mirror.package_name);
                    } else {
                        pb.log('Failed to unmute ' + mirror.package_name);
                    }
                });
            }
        }];

        undo.buttons.push({
            'title': text.get('done'),
            'iconUrl': 'action_tick.png',
            'onclick': function() {
            }
        });

        pb.notifier.show(undo);
    };

    return {
        'title': String.format(text.get('mute_app'), mirror.application_name),
        'short_title': text.get('mute'),
        'iconUrl': 'action_halt.png',
        'onclick': function() {
            mute();
        }
    };
};

var dismissRemote = function(mirror, triggerKey) {
    var dismissal = {
        'type': 'dismissal',
        'package_name': mirror.package_name,
        'notification_id': mirror.notification_id,
        'notification_tag': mirror.notification_tag,
        'source_user_iden': mirror.source_user_iden
    };

    if (mirror.conversation_iden) {
        dismissal.conversation_iden = mirror.conversation_iden;
    }

    if (triggerKey) {
        dismissal.trigger_action = triggerKey;
    }

    pb.post(pb.api + '/v2/ephemerals', {
        'type': 'push',
        'push': dismissal
    }, function(response) {
        if (response) {
            pb.log('Triggered remote dismissal of ' + notificationKey(mirror));
        } else {
            pb.log('Failed to trigger remote dismissal of ' + notificationKey(mirror));
        }
    });
};

var dismissalReceived = function(dismissal) {
    var dismissalKey = notificationKey(dismissal);

    Object.keys(pb.notifier.active).forEach(function(key) {
        if (key.indexOf(dismissalKey) != -1) {
            pb.notifier.dismiss(key);
        }
    });

    delete pendingMirrors[dismissalKey];
};

var notificationKey = function(push) {
    var key = push.package_name + '_' + push.notification_tag + '_' + push.notification_id;
    if (push.conversation_iden) {
        key += '_' + push.conversation_iden;
    }
    return key;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Smart clicks on Android mirrored notifications
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

var androidClickMapping;
var getAndroidClickMapping = function() {
    if (androidClickMapping) {
        return;
    }

    pb.log('Getting Android package name to url mapping');

    androidClickMapping = { };

    var xhr = new XMLHttpRequest();
    xhr.open('GET', 'https://update.pushbullet.com/android_mapping.json', true);
    xhr.onreadystatechange = function() {
        if (xhr.readyState === 4) {
            try {
                androidClickMapping = JSON.parse(xhr.responseText);
            } catch (e) {
                androidClickMapping = null;
            }
        }
    };
    xhr.send();
};

var getWebUrl = function(mirror) {
    if (mirror.package_name == 'com.google.android.gm') {
        var parts = mirror.body.split('\n');
        var email = parts[0];
        return 'https://mail.google.com/mail/u/?authuser=' + email;
    } else if (androidClickMapping) {
        return androidClickMapping[mirror.package_name];
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Power the Quick-Reply popup
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

var openQuickReply = function(mirror) {
    pb.log('Opening quick-reply for ' + notificationKey(mirror));

    var spec = {
        'url': 'quick-reply.html',
        'width': 320,
        'height': 370
    };

    var lastScreenX = localStorage['quickReplyScreenX'];
    var lastScreenY = localStorage['quickReplyScreenY'];
    if (lastScreenX && lastScreenY) {
        spec.top = parseInt(lastScreenY);
        spec.left = parseInt(lastScreenX);
    } else {
        spec.top = Math.floor((window.screen.availHeight / 2) - (spec.height / 2)) - 100,
        spec.left = Math.floor((window.screen.availWidth / 2) - (spec.width / 2)) + 100;
    }

    if (window.chrome) {
        spec.type = 'popup';
        spec.focused = true;

        var listener = function(message, sender, sendResponse) {
            if (message.type == 'quickreply_get_mirror') {
                sendResponse(mirror);
                chrome.runtime.onMessage.removeListener(listener);
            }
        };

        chrome.runtime.onMessage.addListener(listener);

        chrome.windows.create(spec, function(window) {
            chrome.windows.update(window.id, { 'focused': true });
        });
    } else if (window.safari) {
        var listener = function(e) {
            if (e.name == 'quickreply_get_mirror') {
                e.target.page.dispatchMessage('mirror', mirror);
                safari.application.removeEventListener('message', listener, false);
            }
        };

        safari.application.addEventListener('message', listener, false);

        var w = safari.application.openBrowserWindow();
        w.activeTab.url = safari.extension.baseURI + spec.url + '?width=' + spec.width + '&height=' + spec.height;
    } else {
        spec.mirror = mirror;
        self.port.emit('open_quickreply', spec);
    }
};

pb.sendReply = function(mirror, message) {
    pb.post(pb.api + '/v2/ephemerals', {
        'type': 'push',
        'push': {
            'type': 'messaging_extension_reply',
            'package_name': mirror.package_name,
            'source_user_iden': mirror.source_user_iden,
            'target_device_iden': mirror.source_device_iden,
            'conversation_iden': mirror.conversation_iden,
            'message': message
        }
    }, function(response) {
        if (response) {
            pb.log('Forwarding reply to ' + mirror.package_name);
        } else {
            pb.log('Failed to forward reply to ' + mirror.package_name);
        }
    });
};

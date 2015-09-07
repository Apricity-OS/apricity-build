'use strict';

pb.addEventListener('signed_in', function(e) {
    pb.smsQueue = [];
    pb.threads = {};
    pb.thread = {};

    pb.addEventListener('connected', function(e) {
        pb.smsQueue = [];
        pb.threads = {};
        pb.thread = {};

        Object.keys(pb.notifier.active).forEach(function(key) {
            if (key.indexOf('sms') != -1) {
                pb.notifier.dismiss(key);
            }
        });

        pb.dispatchEvent('sms_changed');
    });

    pb.addEventListener('stream_message', function(e) {
        var message = e.detail;
        if (message.type != 'push' || !message.push) {
            return;
        }

        var push = message.push;
        if (push.type != 'sms_changed') {
            return;
        }

        pb.log('SMS changed');

        pb.dispatchEvent('sms_changed');
    });

    pb.addEventListener('locals_changed', function(e) {
        getSmsDevices().forEach(function(device) {
            pb.getThreads(device.iden, function(response) {});
        });
    });

    pb.addEventListener('sms_changed', function(e) {
        var oldThreads = pb.threads;

        pb.threads = {};
        pb.thread = {};

        getSmsDevices().forEach(function(device) {
            pb.getThreads(device.iden, function(response) {
                if (response) {
                    var currentThreads = mapify(response);
                    var previousThreads = mapify(oldThreads[device.iden]);

                    if (!previousThreads) {
                        return;
                    }

                    if (response.dont_notify) {
                        return;
                    }

                    var keySet = {};
                    Object.keys(currentThreads).concat(Object.keys(previousThreads)).forEach(function(key) {
                        keySet[key] = true;
                    });

                    var notifyFor = [];
                    Object.keys(keySet).forEach(function(key) {
                        var current = currentThreads[key];
                        var previous = previousThreads[key];

                        if (!current || !current.latest || current.latest.direction != 'incoming') {
                            return;
                        }

                        if (previous && previous.latest) {
                            if (previous.latest.id == current.latest.id || previous.latest.timestamp > current.latest.timestamp) {
                                return;
                            }
                        }

                        notifyFor.push(current);
                    });

                    if (notifyFor.length < 4) {
                        notifyFor.forEach(function(current) {
                            notifyForSms(device, current);
                        });
                    }
                }
            });
        });
    });
});

var mapify = function(response) {
    if (!response) {
        return null;
    }

    var map = {};
    response.threads.forEach(function(thread) {
        map[thread.id] = thread;
    });

    return map;
};

var notifyForSms = function(device, thread) {
    var chatInfo = pb.findChat(device.iden + '_thread_' + thread.id);
    if (chatInfo && chatInfo.focused && !window.safari) {
        return;
    }

    var name, imageUrl;
    if (thread.recipients.length == 1) {
        var recipient = thread.recipients[0];
        name = recipient.name;
        imageUrl = recipient.thumbnail ? 'data:image/jpeg;base64,' + recipient.thumbnail : 'chip_person.png';
    } else {
        name = thread.recipients.map(function(recipient) { return recipient.name; }).join(', ');
        imageUrl = 'chip_group.png';
    }

    var options = {};
    options.type = 'basic';
    options.key = 'sms' + '_' + device.iden + '_thread_' + thread.id;
    options.iconUrl = imageUrl;
    options.title = name;
    options.message = thread.latest.body;
    options.buttons = [];

    options.contextMessage = String.format(text.get('push_context_message'),
                    name,
                    new Date(Math.floor(thread.latest.timestamp * 1000)).toLocaleTimeString().replace(/:\d+ /, ' '));

    options.onclick = function() {
        openSmsWindow(device, thread);
        sendSmsDismissal();
    };

    if (thread.recipients.length == 1) {
        options.buttons.push({
            'title': text.get('reply'),
            'iconUrl': 'action_sms.png',
            'onclick': function() {
                openSmsWindow(device, thread);
                sendSmsDismissal();
            }
        });
    }

    options.buttons.push({
        'title': text.get('dismiss'),
        'iconUrl': 'action_cancel.png',
        'onclick': function() {
            sendSmsDismissal();
        }
    });


    utils.checkNativeClient(function(response) {
        if (!response) {
            pb.notifier.show(options);   
        }
    });
};

var openSmsWindow = function(device, thread) {
    if (window.safari) {
        pb.openTab(pb.www + '/#sms/' + device.iden + '/' + thread.id);
    } else {
        pb.openChat('sms', device.iden + '_thread_' + thread.id);
    }
};

var sendSmsDismissal = function() {
    var dismissal = {
        'type': 'dismissal',
        'source_user_iden': pb.local.user.iden,
        'package_name': 'sms',
        'notification_id': 0
    };

    pb.post(pb.api + '/v2/ephemerals', {
        'type': 'push',
        'push': dismissal,
        'targets': ['stream', 'android']
    }, function(response) {
        if (response) {
            pb.log('Triggered remote sms dismissal');
        } else {
            pb.log('Failed to trigger remote sms dismissal');
        }
    });
};

var getSmsDevices = function() {
    return utils.asArray(pb.local.devices).filter(function(device) { return device.has_sms; });
};

pb.getThreads = function(deviceIden, callback) {
    if (!pb.threads[deviceIden]) {
        var body = {
            'key': deviceIden + '_threads'
        };

        pb.post(pb.api + '/v3/get-permanent', body, function(response) {
            var data;
            if (response) {
                if (response.data.encrypted) {
                    data = JSON.parse(pb.e2e.decrypt(response.data.ciphertext));
                } else {
                    data = response.data;
                }
            }

            pb.threads[deviceIden] = data;
            callback(pb.threads[deviceIden]);
        });
    } else {
        callback(pb.threads[deviceIden]);
    }
};

pb.getThread = function(deviceIden, threadId, callback) {
    var body = {
        'key': deviceIden+ '_thread_' + threadId
    };

    if (pb.thread[body.key]) {
        callback(pb.thread[body.key]);
    } else {
        pb.post(pb.api + '/v3/get-permanent', body, function(response) {
            var data;
            if (response) {
                if (response.data.encrypted) {
                    data = JSON.parse(pb.e2e.decrypt(response.data.ciphertext));
                } else {
                    data = response.data;
                }
            }

            if (data) {
                data.thread.reverse();
            }

            pb.thread[body.key] = data;
            callback(pb.thread[body.key]);
        });
    }
};

pb.getPhonebook = function(deviceIden, callback) {
    var body = {
        'key': 'phonebook_' + deviceIden,
    };

    pb.post(pb.api + '/v3/get-permanent', body, function(response) {
        var data;
        if (response) {
            if (response.data.encrypted) {
                data = JSON.parse(pb.e2e.decrypt(response.data.ciphertext));
            } else {
                data = response.data;
            }
        }

        callback(data);
    });
};

pb.sendSms = function(deviceIden, threadId, address, body, guid) {
    var sms = {
        'device_iden': deviceIden,
        'thread_id': threadId,
        'address': address,
        'body': body,
        'guid': guid || utils.guid()
    };

    pb.smsQueue.push(sms);

    pb.dispatchEvent('locals_changed');

    processSmsQueue();

    return sms;
};

var processingSms = false;
var processSmsQueue = function() {
    if (processingSms) {
        return;
    }

    var sms = pb.smsQueue[0];
    if (!sms) {
        return;
    }

    processingSms = true;

    var data = {
        'type': 'messaging_extension_reply',
        'source_user_iden': pb.local.user.iden,
        'target_device_iden': sms.device_iden,
        'guid': sms.guid,
        'package_name': 'com.pushbullet.android',
        'conversation_iden': sms.address,
        'message': sms.body
    };

    var push;
    if (pb.e2e.enabled) {
        push = {
            'encrypted' : true,
            'ciphertext': pb.e2e.encrypt(JSON.stringify(data))
        };
    } else {
        push = data;
    }

    pb.post(pb.api + '/v2/ephemerals', {
        'type': 'push',
        'push': push
    }, function(response) {
        pb.smsQueue.shift();

        processingSms = false;

        pb.dispatchEvent('locals_changed');

        processSmsQueue();
    });

    pb.dispatchEvent('active');
};

pb.sendRefreshSms = function(device) {
    var data = {
        'type': 'refresh_sms',
        'source_user_iden': pb.local.user.iden,
        'target_device_iden': device.iden
    };

    var push;
    if (pb.e2e.enabled) {
        push = {
            'encrypted' : true,
            'ciphertext': pb.e2e.encrypt(JSON.stringify(data))
        };
    } else {
        push = data;
    }

    pb.post(pb.api + '/v2/ephemerals', {
        'type': 'push',
        'push': push,
        'targets': ['android']
    }, function(response) {
        pb.log('Sent refresh_sms to ' + device.iden);
    });
};

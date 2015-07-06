'use strict';

if (!self.port && !window.chrome && !window.safari) {
    throw new Error('Shouldn\'t be here');
}

var setUpSmsTab = function() {
    var smsDevices = Object.keys(pb.local.devices).map(function(key) {
        return pb.local.devices[key];
    }).filter(function(device) {
        return device.has_sms;
    }).sort(function(a, b) {
        return a.created - b.created;
    });

    if (smsDevices.length > 0) {
        document.getElementById('sms-tab').style.display = 'block';
    } else {
        // No devices can send SMS, don't load the tab
        return;
    }

    var device = document.getElementById('sms-device');
    var recipient = document.getElementById('sms-recipient');
    var message = document.getElementById('sms-message');

    device.placeholder = text.get('sms_device_placeholder');
    recipient.placeholder = text.get('sms_recipient_placeholder');
    message.placeholder = text.get('message_placeholder');

    picker.setUp({
        'inputId': 'sms-device',
        'pickerId': 'sms-device-picker',
        'overlayId': 'sms-device-overlay',
        'targets': smsDevices,
        'onselect': function(target) {
            pb.getPhonebook(target, function(response) {
                if (response) {
                    var phonebook = response.phonebook.sort(function(a, b) {
                        try {
                            var an = a.name.toLowerCase();
                            var bn = b.name.toLowerCase();
                            if (an > bn) {
                                return 1;
                            } else if (an < bn) {
                                return -1;
                            }
                        } catch (e) { }
                        return 0;
                    });

                    picker.setUp({
                        'inputId': 'sms-recipient',
                        'pickerId': 'sms-recipient-picker',
                        'overlayId': 'sms-recipient-overlay',
                        'targets': phonebook,
                        'noDefault': true,
                        'onselect': function(target) {
                            setTimeout(function() {
                                message.focus();
                            }, 100);
                        }
                    });
                }
            });
        }
    });

    var smsLengthHolder = document.getElementById('sms-length');
    var smsWarning = document.getElementById('sms-warning');

    document.getElementById('sms-warning-link').onclick = function() {
        pb.openTab('https://help.pushbullet.com/articles/why-are-some-text-messages-not-sending/');
    };

    var smsChange = function() {
        var count = utils.smsLength(message.value);

        smsLengthHolder.textContent = count + "/140";

        if (count > 140) {
            smsWarning.style.display = 'inline';
        } else {
            smsWarning.style.display = 'none';
        }
    };

    message.addEventListener('input', function() {
        smsChange();
    }, false);

    message.onkeydown = function(e) {
        if (e.keyCode == utils.ENTER && !e.shiftKey) {
            smsSendButton.onclick();
            return false;
        }
    };

    var smsSendButton = document.getElementById('sms-send-holder');
    smsSendButton.onclick = function() {
        if (message.value.length > 0) {
            sendSms(device.target, recipient.target || recipient.value, message.value);

            setTimeout(function() {
                message.value = '';
                delete localStorage['savedSms'];

                if (window.chrome) {
                    window.close();
                } else if (window.safari) {
                    safari.self.hide();
                }
            }, 120);
        }
    };

    if (localStorage.savedSms) {
        message.value = localStorage.savedSms;
        smsChange();
    }
};

var tearDownSmsTab = function() {
    localStorage.savedSms = document.getElementById('sms-message').value;
};

var sendSms = function(device, recipient, message) {
    if (!device || !recipient || !message) {
        return;
    }

    var push = {
        'type': 'messaging_extension_reply',
        'package_name': 'com.pushbullet.android',
        'source_user_iden': pb.local.user.iden,
        'source_device_iden': device.iden,
        'conversation_iden': recipient.phone || recipient
    };

    pb.sendReply(push, message);

    pb.track({
        'name': 'new_sms_sent',
        'package_name': push.package_name
    });
};

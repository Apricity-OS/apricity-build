'use strict';

var guid, tabId;
window.init = function() {
    guid = utils.getParams(location.search)['guid'];

    if (window.chrome) {
        chrome.runtime.sendMessage({
            'type': 'loopback'
        }, function(response) {
            tabId = response.tabId;
            setUp();
        });
    } else if (self.port) {
        tabId = guid;
        setUp();
    }
};

onFocusChanged = function() {
    updateActiveChat();

    if (focused) {
        localsChangedListener();
        document.getElementById('input').focus();
    }
};

var updateActiveChat = function() {
    var target = document.getElementById('input').target;
    if (target) {
        pb.setActiveChat(tabId, {
            'email': target.with.email_normalized,
            'focused': focused
        });
    } else {
        pb.clearActiveChat(tabId);
    }
};

var setUp = function() {
    setUpPicker();
    setUpInput();

    setUpDropZone('container', function(file) {
        var target = document.getElementById('input').target;
        if (!target) {
            return;
        }

        var push = {
            'email': target.with.email_normalized,
            'file': file
        };

        pb.sendPush(push);
    });

    pb.addEventListener('locals_changed', localsChangedListener);

    monitorAwake();

    var onunload = function() {
        pb.removeEventListener('locals_changed', localsChangedListener);
        pb.setAwake(guid, false);
        pb.clearActiveChat(tabId);
    };

    window.addEventListener('unload', onunload);
};

var localsChangedListener = function(e) {
    if (!window) {
        return;
    }

    var target = document.getElementById('input').target;
    if (target) {
        updateChatUi(target);
    }
};

var setUpPicker = function() {
    var input = document.getElementById('input');

    var chats = utils.asArray(pb.local.chats);
    utils.alphabetizeChats(chats);

    var spec = {
        'inputId': 'chat-target',
        'pickerId': 'chat-picker',
        'overlayId': 'chat-overlay',
        'targets': chats,
        'onselect': function(target) {
            location.hash = target.with.email_normalized;
            input.target = target;
            input.value = '';
            input.focus();
            updateActiveChat();
            updateChatUi(target);
            scrollChat();
        }
    };

    if (location.hash) {
        // Update selected to match the hash
        var email = location.hash.substring(1);
        spec.selectedKey = email;
    }

    picker.setUp(spec);
};

var setUpInput = function() {
    var input = document.getElementById('input');
    input.placeholder = text.get('message_placeholder_files');

    input.addEventListener('input', function(e) {
        reportAwake();
    });

    input.onkeydown = function(e) {
        if (e.keyCode == utils.ENTER && !e.shiftKey) {
            if (!input.target) {
                return false;
            }

            if (!input.value) {
                return false;
            }

            var push = {
                'email': input.target.with.email_normalized
            };

            if (input.value) {
                if (utils.isLink(input.value)) {
                    push.type = 'link';
                    push.url = input.value;
                } else {
                    push.type = 'note';
                    push.body = input.value;
                }
            }

            pb.sendPush(push);

            input.value = '';

            scrollChat();

            reportAwake();

            return false;
        }
    };

    input.focus();
};

var monitorAwake = function() {
    pb.setAwake(guid, true);

    document.body.onmousemove = function(e) {
        if (window.mouseLastX !== e.clientX || window.mouseLastY !== e.clientY) {
            reportAwake();
        }

        window.mouseLastX = e.clientX;
        window.mouseLastY = e.clientY;
    };
};

var lastReportedAwake = Date.now();
var reportAwake = function() {
    if (Date.now() - lastReportedAwake > 10 * 1000) {
        lastReportedAwake = Date.now();
        pb.setAwake(guid, true);
    }
};

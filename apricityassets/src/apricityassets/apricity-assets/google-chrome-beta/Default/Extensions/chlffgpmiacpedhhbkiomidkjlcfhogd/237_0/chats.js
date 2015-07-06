'use strict';

var activeChats = {};

pb.addEventListener('signed_in', function() {
    activeChats = {};
});

pb.setActiveChat = function(tabId, info) {
    pb.log('Opened/updated chat ' + tabId + ', email=' + info.email + ', focused=' + info.focused);

    activeChats[tabId] = info;
};

pb.clearActiveChat = function(tabId) {
    pb.log('Closed chat ' + tabId);

    delete activeChats[tabId];
};

pb.openChat = function(email) {
    var found = false;
    Object.keys(activeChats).forEach(function(tabId) {
        var info = activeChats[tabId];
        if (!info) {
            return;
        } else if (info.email != email) {
            return;
        } else if (tabId == 'panel') {
            return;
        }

        found = true;

        focusChat(tabId);
    });

    if (!found) {
        openChat(email);
    }
};

var openChat = function(email) {
    var spec = {
        'url': 'chat.html?guid=' + utils.guid() + '#' + email,
        'width': 320,
        'height': 420
    };

    if (window.chrome) {
        spec.type = 'popup';
        spec.focused = true;

        chrome.windows.create(spec, function(created) {
            chrome.windows.update(created.id, { 'focused': true }, function() {
            });
        });
    } else if (window.safari) {
    } else {
        self.port.emit('open_chat', email);
    }

    pb.track({
        'name': 'chat_window_opened'
    });
};

var focusChat = function(tabId) {
    if (window.chrome) {
        chrome.tabs.get(parseInt(tabId), function(tab) {
            chrome.windows.update(tab.windowId, { 'focused': true }, function() {
            });
        });
    } else if (window.safari) {
    } else {
        self.port.emit('focus_chat', tabId);
    }
};

pb.findChat = function(email) {
    var chatTabInfo;
    Object.keys(activeChats).forEach(function(tabId) {
        var info = activeChats[tabId];
        if (info.email == email) {
            if (chatTabInfo && chatTabInfo.focused && !info.focused) {
                // We've already found a focused chat, don't clobber with this not-focused one
                return;
            }

            chatTabInfo = info;
        }
    });

    return chatTabInfo;
};

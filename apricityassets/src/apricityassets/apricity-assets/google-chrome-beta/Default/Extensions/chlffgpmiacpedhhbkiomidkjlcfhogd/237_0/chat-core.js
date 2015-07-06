'use strict';

var alreadyLoaded = false;

var updateInterval;
var updateChatUi = function(target) {
    var historyLink = pb.www;

    if (target.with) {
        historyLink += '/#people/' + target.with.email_normalized;
    } else if (target.tag) {
        historyLink += '/#following/' + target.tag;
    } else {
        historyLink += '/#people/me/';
    }

    if (!alreadyLoaded) {
        setTimeout(function() {
            alreadyLoaded = true;
            drawChat(filterAndSortPushes(target), targets(), clearFailed, retryFailed, cancelUpload, historyLink);
        }, 300);
    } else {
        drawChat(filterAndSortPushes(target), targets(), clearFailed, retryFailed, cancelUpload, historyLink);
    }

    clearInterval(updateInterval);
    updateInterval = setInterval(function() {
        if (window) {
            drawChat(filterAndSortPushes(target), targets(), clearFailed, retryFailed, cancelUpload, historyLink);
        }
    }, 60 * 1000);
};

var filterAndSortPushes = function(target) {
    var pushesFilter = function(push) {
        if (target.with) {
            return push.sender_email_normalized == target.with.email_normalized
                   || push.receiver_email_normalized == target.with.email_normalized
                   || push.email == target.with.email_normalized;
        } else if (target.tag) {
            return push.channel_iden == target.iden
                   || push.channel_tag == target.tag;
        } else {
            if (target.iden == '*') {
                return (push.direction == 'self' && !push.client_iden)
                       || (!push.direction && !push.email && !push.channel_tag);
            } else {
                return (push.direction == 'self' && !push.client_iden && (push.source_device_iden == target.iden || push.target_device_iden == target.iden))
                       || (!push.direction && !push.email && !push.channel_tag && push.device_iden == target.iden);
            }
        }

        return push;
    };

    var pushes = utils.asArray(pb.local.pushes).sort(function(a, b) {
        return a.created - b.created;
    }).filter(pushesFilter);

    var pendingPushes = utils.asArray(pb.successfulPushes).concat(pb.pushQueue).filter(function(pending) {
        for (var i = 0; i < pushes.length; i++) {
            var push = pushes[i];
            if (pending.guid == push.guid) {
                return false;
            }
        }
        return true;
    });

    pushes = pushes.concat(pendingPushes).concat(pb.fileQueue).concat(pb.failedPushes).filter(pushesFilter);

    var modified = false;
    pushes.forEach(function(push) {
        if (pb.awake && focused && push.direction == 'incoming') {
            if (!push.dismissed) {
                pb.markDismissed(push);
            } else if (push.awake_app_guids && push.awake_app_guids.indexOf('extension-' + localStorage.client_id) != -1) {
                delete push.awake_app_guids;
                if (window.chrome || window.safari) {
                    modified = true;
                    pb.notifier.dismiss(pb.groupKey(push));
                } else {
                    self.port.emit('clear_awake_app_guids', push.iden);
                }
            }
        }
    });

    if (modified) {
        pb.savePushes();
    }

    return pushes;
};

var targets = function() {
    return {
        'devices': utils.asArray(pb.local.devices),
        'chats': utils.asArray(pb.local.chats),
        'channels': utils.asArray(pb.local.channels),
        'subscriptions': utils.asArray(pb.local.subscriptions),
        'device': pb.local.device,
        'me': pb.local.user
    };
};

var retryFailed = function(push) {
    delete push.failed;
    delete push.error;
    pb.sendPush(push);
};

var clearFailed = function(push) {
    pb.clearFailed(push);
};

var cancelUpload = function(push) {
    pb.cancelUpload(push);
};

var setUpDropZone = function(containerId, ondrop) {
    var chatBar = document.getElementById(containerId);
    var dropZone = document.getElementById('chat-drop-zone');

    chatBar.addEventListener('dragenter', function(e) {
        e.stopPropagation();
        e.preventDefault();
        e.dataTransfer.dropEffect = 'copy';

        dropZone.style.display = 'block';
    });

    dropZone.addEventListener('dragover', function(e) {
        e.stopPropagation();
        e.preventDefault();
    });

    dropZone.addEventListener('dragleave', function(e) {
        if (e.toElement.id == 'chat-drop-zone') {
            e.stopPropagation();
            e.preventDefault();

            dropZone.style.display = 'none';
        }
    });

    dropZone.addEventListener('drop', function(e) {
        e.stopPropagation();
        e.preventDefault();

        dropZone.style.display = 'none';

        var files = e.dataTransfer.files;
        for (var i = 0; i < files.length; i++) {
            var file = files[i];
            ondrop(file);
        }
    });

    document.addEventListener('paste', function(e) {
        var items = (e.clipboardData || e.originalEvent.clipboardData).items;
        for (var i = 0; i < items.length; i++) {
            var item = items[i];
            if (item.kind == 'file') {
                var file = item.getAsFile();
                e.stopPropagation();
                e.preventDefault();
                ondrop(file);
            }
        }
    });
};

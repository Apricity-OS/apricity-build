'use strict';

pb.local = {};

var types = ['devices', 'chats', 'grants', 'subscriptions', 'channels', 'pushes'];

pb.addEventListener('signed_in', function() {
    pb.syncing = {};

    types.forEach(function(type) {
        try {
            pb.local[type] = localStorage[type] ? JSON.parse(localStorage[type]) : {};
        } catch (e) {
            pb.local[type] = {};
            delete localStorage[type + 'Bootstrapped'];
            delete localStorage[type];
        }
    });

    pb.addEventListener('connected', function() {
        pb.sync();
    });

    pb.addEventListener('stream_message', function(e) {
        var message = e.detail;
        if (message.type == 'tickle') {
            pb.sync();
        }
    });

    pb.addEventListener('locals_changed', function(e) {
        var guids = Object.keys(pb.successfulPushes);
        if (guids.length > 0) {
            utils.asArray(pb.local.pushes).forEach(function(push) {
                if (guids.indexOf(push.guid) != -1) {
                    delete pb.successfulPushes[push.guid];
                }
            });
        }
    });
});

pb.addEventListener('signed_out', function(e) {
    pb.syncing = {};
    pb.local = {};
});

pb.sync = function(backoff) {
    var bootstrapped = true;

    types.forEach(function(type) {
        if (!localStorage[type + 'Bootstrapped']) {
            bootstrapped = false;
            syncInternal(type, backoff);
        }
    });

    if (bootstrapped) {
        syncInternal('everything', backoff);
    }
};

var syncInternal = function(type, backoff) {
    if (!type) {
        type = 'everything';
    }

    if (!backoff) {
        backoff = 10 * 1000;
    }

    if (pb.syncing[type]) {
        pb.pendingSync = true;
        return;
    }

    pb.syncing[type] = true;
    delete pb.pendingSync;
    clearTimeout(pb.syncTimeout);

    var modifiedAfter = type == 'everything' ? parseFloat(localStorage['modifiedAfter']) || 0 : 0;
    var cursor = localStorage[type + 'Cursor'];
    var url = pb.api + '/v2/' + type;

    if (cursor) {
        url += '?cursor=' + cursor;
    } else {
        url += '?modified_after=' + modifiedAfter;
    }

    if (modifiedAfter == 0) {
        url += '&active_only=true';
    }

    pb.get(url, function(response) {
        delete pb.syncing[type];

        var cursor;
        if (response) {
            cursor = type == 'pushes' ? '' : response.cursor || '';
            localStorage[type + 'Cursor'] = cursor;

            if (type == 'everything') {
                types.forEach(function(type) {
                    var syncables = response[type];
                    ingest(type, syncables, cursor);
                });
            } else {
                ingest(type, response[type], cursor);

                if (!cursor) {
                    localStorage[type + 'Bootstrapped'] = true;
                }
            }

            pb.dispatchEvent('locals_changed');
        } else {
            clearTimeout(pb.syncTimeout);

            if (!pb.pendingSync) {
                pb.log('Sync failed, scheduling retry');

                pb.syncTimeout = setTimeout(function() {
                    pb.sync(Math.min(backoff * 2, 10 * 60 * 1000));
                }, backoff);
            }
        }

        if (pb.pendingSync || cursor) {
            pb.sync(backoff);
        }
    });
};

var ingest = function(type, syncables, cursor) {
    var locals = pb.local[type];

    syncables.forEach(function(syncable) {
        if (syncable.active && syncable.pushable !== false) {
            locals[syncable.iden] = syncable;
        } else {
            delete locals[syncable.iden];
        }

        localStorage['modifiedAfter'] = Math.max(syncable.modified, parseFloat(localStorage['modifiedAfter']) || 0);
    });

    if (type == 'pushes') {
        var keys = Object.keys(locals);
        if (keys.length > 500) {
            var pushes = utils.asArray(pb.local.pushes).sort(function(a, b) {
                return b.modified - a.modified;
            });

            locals = { };
            pushes.slice(0, 500).forEach(function(push) {
                locals[push.iden] = push;
                
                // Fix link urls
                if (push.url && push.url.indexOf('://') == -1) {
                    push.url = 'http://' + push.url;
                }
            });
        }

        pb.savePushes();
    } else {
        localStorage[type] = JSON.stringify(locals);
    }

    pb.local[type] = locals;

    if (!cursor) {
        pb.dispatchEvent(type + '_ready');
    }
};

pb.savePushes = function() {
    pb.log('Saving pushes');
    localStorage.pushes = JSON.stringify(pb.local.pushes);
};

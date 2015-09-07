'use strict';

pb.notifier.notify = function(options) {
    if (!window.Notification) {
        pb.log('Notifications not available');
        return;
    }

    if (pb.isSnoozed()) {
        pb.log('Not showing notification ' + options.key + ', snoozed');

        pb.notifier.active[options.key] = options;
        return;
    }

    var notification = new Notification(options.title, {
        'body': options.message,
        'tag': options.key
    });

    notification.onclick = function() {
        if (options.onclick) {
            options.onclick();
        }

        notification.close();
    };

    notification.onclose = function() {
        if (options.onclose) {
            options.onclose();
        }

        delete pb.notifier.active[options.key];
        
        pb.dispatchEvent('notifications_changed');
    };

    options.notification = notification;
};

pb.notifier.dismiss = function(key) {
    var options = pb.notifier.active[key];
    if (options) {
        options.notification.close();
    }
};

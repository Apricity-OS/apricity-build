'use strict';

pb.notifier.notify = function(options) {
    if (pb.isSnoozed()) {
        pb.log('Not showing notification ' + options.key + ', snoozed');

        pb.notifier.active[options.key] = options;
        return;
    }

    pb.notifier.active[options.key] = options;

    var spec = {
        'key': options.key,
        'title': options.title,
        'message': options.message,
        'contextMessage': options.contextMessage,
        'iconUrl': options.iconUrl
    };

    if (spec.message.length > 500) {
        spec.message = spec.message.substring(0, 500);
    }

    self.port.emit('show_notification', spec);

    pb.dispatchEvent('notifications_changed');
};

pb.notifier.dismiss = function(key) {
    var options = pb.notifier.active[key];
    if (options && options.onclose) {
        options.onclose();
    }

    delete pb.notifier.active[key];

    pb.dispatchEvent('notifications_changed');
};

'use strict';

pb.notifier = {
    'active': {}
};

pb.notifier.show = function(options) {
    pb.log('Showing notification with key ' + options.key);

    options.allButtons = options.buttons;
    options.fullMessage = options.message;
    options.allItems = options.items;

    if (pb.settings.onlyShowTitles) {
        if (options.type == 'list') {
            options.items = [];
        }

        options.message = '';
    }

    pb.notifier.notify(options);

    pb.dispatchEvent('active');
};

pb.notifier.dismiss = function(key) {
    // Stub, overwritten in notifications-chrome.js etc.
};

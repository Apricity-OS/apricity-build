chrome.commands.onCommand.addListener(function (command) {
    utils.wrap(function () {
        pb.log('Receieved command ' + command);

        if (command == 'dismiss-most-recent-notification') {



            if (Object.keys(pb.notifier.active).length > 0) {
                var sortedKeys = [], key;
                for (key in pb.notifier.active) {
                    sortedKeys.push(key);
                }

                sortedKeys.sort(function(a, b) {
                    return pb.notifier.active[b].created - pb.notifier.active[a].created;
                });

                key = sortedKeys[0];
                var notification = pb.notifier.active[key];

                pb.log('Dismissing ' + key + ' by keyboard shortcut');

                pb.notifier.dismiss(key);
            }
        }

        pb.track({
            'name': 'keyboard_shortcut',
            'command': command
        });
    });
});

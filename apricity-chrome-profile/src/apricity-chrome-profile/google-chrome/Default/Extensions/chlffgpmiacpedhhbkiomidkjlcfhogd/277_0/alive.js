'use strict';

var reportAlive = function() {
    checkClipboardPermissions(function(granted) {
        pb.trackPerDay({
            'name': 'alive',
            'signed_in': !!localStorage.apiKey,
            'language': text.languageCode,
            'clipboard_sync': granted,
            'e2e_enabled': pb.e2e.enabled
        });
    });
};

setTimeout(function() {
    reportAlive();
}, 30 * 1000); // After 30 seconds, giving bootstrapping time to finish but not bound to events in case we're not signed in

setInterval(function() {
    reportAlive();
}, 12 * 60 * 60 * 1000); // Every 12 hours

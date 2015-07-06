'use strict';

var reportAlive = function() {
    pb.trackPerDay({
        'name': 'alive',
        'signed_in': !!localStorage.apiKey
    });
};

setTimeout(function() {
    reportAlive();
}, 30 * 1000); // After 30 seconds, giving bootstrapping time to finish but not bound to events (in case we're not signed in)

setInterval(function() {
    reportAlive();
}, 1 * 60 * 60 * 1000); // Every hour

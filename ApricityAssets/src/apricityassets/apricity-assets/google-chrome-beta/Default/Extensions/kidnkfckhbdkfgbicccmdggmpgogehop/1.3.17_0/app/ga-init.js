// If you are using the Closure compiler:
//goog.require('analytics.getService');

// You'll usually only ever have to create one service instance.
var service = analytics.getService('maxiang');

// You can create as many trackers as you want. Each tracker has its own state
// independent of other tracker instances.
var tracker = service.getTracker('UA-43559825-3');  // Supply your GA Tracking ID.
tracker.sendAppView('MainView');


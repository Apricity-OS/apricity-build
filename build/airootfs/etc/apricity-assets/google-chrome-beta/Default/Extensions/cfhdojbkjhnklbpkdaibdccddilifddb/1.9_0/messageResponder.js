/*
 * This file is part of Adblock Plus <https://adblockplus.org/>,
 * Copyright (C) 2006-2015 Eyeo GmbH
 *
 * Adblock Plus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Adblock Plus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Adblock Plus.  If not, see <http://www.gnu.org/licenses/>.
 */

(function(global)
{
  if (!global.ext)
    global.ext = require("ext_background");

  var Utils = require("utils").Utils;
  var FilterStorage = require("filterStorage").FilterStorage;
  var FilterNotifier = require("filterNotifier").FilterNotifier;
  var defaultMatcher = require("matcher").defaultMatcher;
  var BlockingFilter = require("filterClasses").BlockingFilter;
  var Synchronizer = require("synchronizer").Synchronizer;

  var subscriptionClasses = require("subscriptionClasses");
  var Subscription = subscriptionClasses.Subscription;
  var DownloadableSubscription = subscriptionClasses.DownloadableSubscription;
  var SpecialSubscription = subscriptionClasses.SpecialSubscription;

  var subscriptionKeys = ["disabled", "homepage", "lastSuccess", "title", "url", "downloadStatus"];
  function convertSubscription(subscription)
  {
    var result = {};
    for (var i = 0; i < subscriptionKeys.length; i++)
      result[subscriptionKeys[i]] = subscription[subscriptionKeys[i]]
    return result;
  }

  var changeListeners = null;
  var messageTypes = {
    "app": "app.listen",
    "filter": "filters.listen",
    "subscription": "subscriptions.listen"
  };

  function onFilterChange(action)
  {
    var parts = action.split(".", 2);
    var type;
    if (parts.length == 1)
    {
      type = "app";
      action = parts[0];
    }
    else
    {
      type = parts[0];
      action = parts[1];
    }

    if (!messageTypes.hasOwnProperty(type))
      return;

    var args = Array.prototype.slice.call(arguments, 1).map(function(arg)
    {
      if (arg instanceof Subscription)
        return convertSubscription(arg);
      else
        return arg;
    });

    var pages = changeListeners.keys();
    for (var i = 0; i < pages.length; i++)
    {
      var filters = changeListeners.get(pages[i]);
      if (filters[type] && filters[type].indexOf(action) >= 0)
      {
        pages[i].sendMessage({
          type: messageTypes[type],
          action: action,
          args: args
        });
      }
    }
  };

  global.ext.onMessage.addListener(function(message, sender, callback)
  {
    switch (message.type)
    {
      case "app.get":
        if (message.what == "issues")
        {
          var info = require("info");
          callback({
            seenDataCorruption: "seenDataCorruption" in global ? global.seenDataCorruption : false,
            filterlistsReinitialized: "filterlistsReinitialized" in global ? global.filterlistsReinitialized : false,
            legacySafariVersion: (info.platform == "safari" && (
                Services.vc.compare(info.platformVersion, "6.0") < 0 ||   // beforeload breaks websites in Safari 5
                Services.vc.compare(info.platformVersion, "6.1") == 0 ||  // extensions are broken in 6.1 and 7.0
                Services.vc.compare(info.platformVersion, "7.0") == 0))
          });
        }
        else if (message.what == "doclink")
          callback(Utils.getDocLink(message.link));
        else if (message.what == "localeInfo")
        {
          var bidiDir;
          if ("chromeRegistry" in Utils)
            bidiDir = Utils.chromeRegistry.isLocaleRTL("adblockplus") ? "rtl" : "ltr";
          else
            bidiDir = ext.i18n.getMessage("@@bidi_dir");

          callback({locale: Utils.appLocale, bidiDir: bidiDir});
        }
        else
          callback(null);
        break;
      case "app.open":
        if (message.what == "options")
          ext.showOptions();
        break;
      case "subscriptions.get":
        var subscriptions = FilterStorage.subscriptions.filter(function(s)
        {
          if (message.ignoreDisabled && s.disabled)
            return false;
          if (s instanceof DownloadableSubscription && message.downloadable)
            return true;
          if (s instanceof SpecialSubscription && message.special)
            return true;
          return false;
        });
        callback(subscriptions.map(convertSubscription));
        break;
      case "filters.blocked":
        var filter = defaultMatcher.matchesAny(message.url, message.requestType, message.docDomain, message.thirdParty);
        callback(filter instanceof BlockingFilter);
        break;
      case "subscriptions.toggle":
        var subscription = Subscription.fromURL(message.url);
        if (subscription.url in FilterStorage.knownSubscriptions && !subscription.disabled)
          FilterStorage.removeSubscription(subscription);
        else
        {
          subscription.disabled = false;
          subscription.title = message.title;
          subscription.homepage = message.homepage;
          FilterStorage.addSubscription(subscription);
          if (!subscription.lastDownload)
            Synchronizer.execute(subscription);
        }
        break;
      case "subscriptions.listen":
        if (!changeListeners)
        {
          changeListeners = new global.ext.PageMap();
          FilterNotifier.addListener(onFilterChange);
        }

        var filters = changeListeners.get(sender.page);
        if (!filters)
        {
          filters = Object.create(null);
          changeListeners.set(sender.page, filters);
        }

        if (message.filter)
          filters.subscription = message.filter;
        else
          delete filters.subscription;
        break;
    }
  });
})(this);

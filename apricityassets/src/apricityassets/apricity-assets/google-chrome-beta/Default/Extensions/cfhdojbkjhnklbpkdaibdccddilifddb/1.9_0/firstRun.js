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

"use strict";

(function()
{
  function E(id)
  {
    return document.getElementById(id);
  }

  // Load subscriptions for features
  var featureSubscriptions = [
    {
      feature: "malware",
      homepage: "http://malwaredomains.com/",
      title: "Malware Domains",
      url: "https://easylist-downloads.adblockplus.org/malwaredomains_full.txt"
    },
    {
      feature: "social",
      homepage: "https://www.fanboy.co.nz/",
      title: "Fanboy's Social Blocking List",
      url: "https://easylist-downloads.adblockplus.org/fanboy-social.txt"
    },
    {
      feature: "tracking",
      homepage: "https://easylist.adblockplus.org/",
      title: "EasyPrivacy",
      url: "https://easylist-downloads.adblockplus.org/easyprivacy.txt"
    }
  ];

  function getDocLink(link, callback)
  {
    ext.backgroundPage.sendMessage({
      type: "app.get",
      what: "doclink",
      link: link
    }, callback);
  }

  function onDOMLoaded()
  {
    // Set up logo image
    var logo = E("logo");
    logo.src = "skin/abp-128.png";
    var errorCallback = function()
    {
      logo.removeEventListener("error", errorCallback, false);
      // We are probably in Chrome/Opera/Safari, the image has a different path.
      logo.src = "icons/detailed/abp-128.png";
    };
    logo.addEventListener("error", errorCallback, false);

    // Set up URLs
    getDocLink("donate", function(link)
    {
      E("donate").href = link;
    });

    getDocLink("contributors", function(link)
    {
      E("contributors").href = link;
    });

    getDocLink("acceptable_ads_criteria", function(link)
    {
      setLinks("acceptableAdsExplanation", link, openFilters);
    });

    getDocLink("contribute", function(link)
    {
      setLinks("share-headline", link);
    });

    ext.backgroundPage.sendMessage({
      type: "app.get",
      what: "issues"
    }, function(issues)
    {
      // Show warning if data corruption was detected
      if (issues.seenDataCorruption)
      {
        E("dataCorruptionWarning").removeAttribute("hidden");
        getDocLink("knownIssuesChrome_filterstorage", function(link)
        {
          setLinks("dataCorruptionWarning", link);
        });
      }

      // Show warning if filterlists settings were reinitialized
      if (issues.filterlistsReinitialized)
      {
        E("filterlistsReinitializedWarning").removeAttribute("hidden");
        setLinks("filterlistsReinitializedWarning", openFilters);
      }

      if (issues.legacySafariVersion)
        E("legacySafariWarning").removeAttribute("hidden");
    });

    // Set up feature buttons linked to subscriptions
    featureSubscriptions.forEach(initToggleSubscriptionButton);
    updateToggleButtons();
    updateSocialLinks();

    ext.onMessage.addListener(function(message)
    {
      if (message.type == "subscriptions.listen")
      {
        updateToggleButtons();
        updateSocialLinks();
      }
    });
    ext.backgroundPage.sendMessage({
      type: "subscriptions.listen",
      filter: ["added", "removed", "updated", "disabled"]
    });
  }

  function initToggleSubscriptionButton(featureSubscription)
  {
    var feature = featureSubscription.feature;

    var element = E("toggle-" + feature);
    element.addEventListener("click", function(event)
    {
      ext.backgroundPage.sendMessage({
        type: "subscriptions.toggle",
        url: featureSubscription.url,
        title: featureSubscription.title,
        homepage: featureSubscription.homepage
      });
    }, false);
  }

  function openSharePopup(url)
  {
    var iframe = E("share-popup");
    var glassPane = E("glass-pane");
    var popupMessageReceived = false;

    var popupMessageListener = function(event)
    {
      if (!/[.\/]adblockplus\.org$/.test(event.origin))
        return;

      var width = event.data.width;
      var height = event.data.height;
      iframe.width = width;
      iframe.height = height;
      iframe.style.marginTop = -height/2 + "px";
      iframe.style.marginLeft = -width/2 + "px";
      popupMessageReceived = true;
      window.removeEventListener("message", popupMessageListener);
    };
    // Firefox requires last parameter to be true to be triggered by unprivileged pages
    window.addEventListener("message", popupMessageListener, false, true);

    var popupLoadListener = function()
    {
      if (popupMessageReceived)
      {
        iframe.className = "visible";

        var popupCloseListener = function()
        {
          iframe.className = glassPane.className = "";
          document.removeEventListener("click", popupCloseListener);
        };
        document.addEventListener("click", popupCloseListener, false);
      }
      else
      {
        glassPane.className = "";
        window.removeEventListener("message", popupMessageListener);
      }

      iframe.removeEventListener("load", popupLoadListener);
    };
    iframe.addEventListener("load", popupLoadListener, false);

    iframe.src = url;
    glassPane.className = "visible";
  }

  function updateSocialLinks()
  {
    var networks = ["twitter", "facebook", "gplus"];
    networks.forEach(function(network)
    {
      var link = E("share-" + network);
      var message = {
        type: "filters.blocked",
        url: link.getAttribute("data-script"),
        requestType: "SCRIPT",
        docDomain: "adblockplus.org",
        thirdParty: true
      };
      ext.backgroundPage.sendMessage(message, function(blocked)
      {
        // Don't open the share page if the sharing script would be blocked
        if (blocked)
          link.removeEventListener("click", onSocialLinkClick, false);
        else
          link.addEventListener("click", onSocialLinkClick, false);
      });
    });
  }

  function onSocialLinkClick(event)
  {
    event.preventDefault();

    getDocLink(event.target.id, function(link)
    {
      openSharePopup(link);
    });
  }

  function setLinks(id)
  {
    var element = E(id);
    if (!element)
    {
      return;
    }

    var links = element.getElementsByTagName("a");

    for (var i = 0; i < links.length; i++)
    {
      if (typeof arguments[i + 1] == "string")
      {
        links[i].href = arguments[i + 1];
        links[i].setAttribute("target", "_blank");
      }
      else if (typeof arguments[i + 1] == "function")
      {
        links[i].href = "javascript:void(0);";
        links[i].addEventListener("click", arguments[i + 1], false);
      }
    }
  }

  function openFilters()
  {
    ext.backgroundPage.sendMessage({type: "app.open", what: "options"});
  }

  function updateToggleButtons()
  {
    ext.backgroundPage.sendMessage({
      type: "subscriptions.get",
      downloadable: true,
      ignoreDisabled: true
    }, function(subscriptions)
    {
      var known = Object.create(null);
      for (var i = 0; i < subscriptions.length; i++)
        known[subscriptions[i].url] = true;
      for (var i = 0; i < featureSubscriptions.length; i++)
      {
        var featureSubscription = featureSubscriptions[i];
        updateToggleButton(featureSubscription.feature, featureSubscription.url in known);
      }
    });
  }

  function updateToggleButton(feature, isEnabled)
  {
    var button = E("toggle-" + feature);
    if (isEnabled)
      button.classList.remove("off");
    else
      button.classList.add("off");
  }

  document.addEventListener("DOMContentLoaded", onDOMLoaded, false);
})();

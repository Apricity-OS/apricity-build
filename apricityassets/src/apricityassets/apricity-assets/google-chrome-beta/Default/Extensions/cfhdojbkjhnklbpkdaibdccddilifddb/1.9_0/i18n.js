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

// This variable should no longer be necessary once options.js in Chrome
// accesses ext.i18n directly.
var i18n = ext.i18n;

// Getting UI locale cannot be done synchronously on Firefox,
// requires messaging the background page. For Chrome and Safari,
// we could get the UI locale here, but would need to duplicate
// the logic implemented in Utils.appLocale.
ext.backgroundPage.sendMessage(
  {
    type: "app.get",
    what: "localeInfo"
  },
  function(localeInfo)
  {
    document.documentElement.lang = localeInfo.locale;
    document.documentElement.dir = localeInfo.bidiDir;
  }
);

// Inserts i18n strings into matching elements. Any inner HTML already in the element is
// parsed as JSON and used as parameters to substitute into placeholders in the i18n
// message.
ext.i18n.setElementText = function(element, stringName, arguments)
{
  function processString(str, element)
  {
    var match = /^(.*?)<(a|strong)>(.*?)<\/\2>(.*)$/.exec(str);
    if (match)
    {
      processString(match[1], element);

      var e = document.createElement(match[2]);
      processString(match[3], e);
      element.appendChild(e);

      processString(match[4], element);
    }
    else
      element.appendChild(document.createTextNode(str));
  }

  while (element.lastChild)
    element.removeChild(element.lastChild);
  processString(ext.i18n.getMessage(stringName, arguments), element);
}

// Loads i18n strings
function loadI18nStrings()
{
  var nodes = document.querySelectorAll("[class^='i18n_']");
  for(var i = 0; i < nodes.length; i++)
  {
    var node = nodes[i];
    var arguments = JSON.parse("[" + node.textContent + "]");
    if (arguments.length == 0)
      arguments = null;

    var className = node.className;
    if (className instanceof SVGAnimatedString)
      className = className.animVal;
    var stringName = className.split(/\s/)[0].substring(5);

    ext.i18n.setElementText(node, stringName, arguments);
  }
}

// Provides a more readable string of the current date and time
function i18n_timeDateStrings(when)
{
  var d = new Date(when);
  var timeString = d.toLocaleTimeString();

  var now = new Date();
  if (d.toDateString() == now.toDateString())
    return [timeString];
  else
    return [timeString, d.toLocaleDateString()];
}

// Fill in the strings as soon as possible
window.addEventListener("DOMContentLoaded", loadI18nStrings, true);

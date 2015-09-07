/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

require.scopes.info = {
  get addonID()
  {
    return chrome.i18n.getMessage("@@extension_id");
  },
  addonName: "adblockpluschrome",
  addonVersion: "1.9.1",
  addonRoot: "",

  application: "chrome",
  get applicationVersion()
  {
    return this.platformVersion;
  },

  platform: "chromium",
  get platformVersion()
  {
    var match = /\bChrome\/(\S+)/.exec(navigator.userAgent);
    return (match ? match[1] : "0");
  }
};
'use strict';

if (!self.port && !window.chrome && !window.safari) {
    throw new Error('Shouldn\'t be here');
}

var textMappings = {
    'sign-in': 'sign_in',
    'sign-out': 'sign_out',
    'options': 'options',
    'people-tab': 'people',
    'devices-tab': 'devices',
    'sms-tab': 'sms',
    'notifications-tab': 'notifications',
    'sign-up': 'panel_sign_up',
    'cant-sign-in': 'panel_cant_sign_in',
    'third-party-cookies': 'panel_third_party_cookies',
    'sms-disclaimer': 'sms_disclaimer',
    'sms-warning': 'sms_warning',
    'notifications-empty-text': 'no_notifications',
    'device-picker-label': 'recipient_placeholder',
    'sms-device-picker-label': 'device',
    'sms-recipient-picker-label': 'recipient_placeholder'
};

var tabId;
window.init = function() {
    if (!location.hash) {
        tabId = 'panel';
        setUp();
    } else {
        if (window.chrome) {
            chrome.runtime.sendMessage({
                'type': 'loopback'
            }, function(response) {
                tabId = response.tabId;
                setUp();
            });
        } else {
            tabId = 'popout';
            setUp();
        }
    }
};

var setUp = function() {
    Object.keys(textMappings).forEach(function(key) {
        document.getElementById(key).textContent = text.get(textMappings[key]); 
    });

    var goToSite = function() {
        pb.openTab(pb.www);
        if (window.safari) {
            safari.self.hide();
        }
    };

    document.getElementById('logo-link').onclick = goToSite;
    document.getElementById('bullet-link').onclick = goToSite;
    
    if (window.safari) {
        safari.self.height = 451;
        safari.self.width = 640;
    }

    var signIn = function() {
        pb.openTab('https://www.pushbullet.com/signin');
        if (window.safari) {
            safari.self.hide();
        }
    };

    document.getElementById('sign-in').onclick = function() {
        signIn();
    };

    document.getElementById('sign-up').onclick = function() {
        signIn();
    };

    document.getElementById('third-party-cookies').onclick = function() {
        pb.openTab('https://support.mozilla.org/en-US/kb/disable-third-party-cookies');
    };

    if (!pb.local.user) {
        return;
    }

    setUpSettingsMenu();
    setUpPopout();
    setUpSmsTab();
    setUpNotificationsTab();
    setUpTabs();
    checkNativeClient();
    monitorAwake();

    if (location.hash) {
        document.body.classList.add(location.hash.substring(1));
    }

    window.addEventListener('unload', onunload);
};

if (self.port) {
    self.port.on('hidden', function() {
        onunload();
    });
}

var onunload = function() {
    tearDownPeopleTab();
    tearDownSmsTab();
    tearDownNotificationsTab();

    pb.setAwake(location.hash ? location.hash.substring(1) : 'panel', false);
};

var setUpSettingsMenu = function() {
    var settingsGear = document.getElementById('settings-gear');
    var settingsMenu = document.getElementById('settings-menu');
    var menuSink = document.getElementById('menu-sink');

    settingsGear.onclick = function() {
        settingsMenu.style.display = 'block';
        menuSink.style.display = 'block';
    };

    menuSink.onclick = function() {
        settingsMenu.style.display = 'none';
        menuSink.style.display = 'none';
    };

    var snooze = document.getElementById('snooze');

    var setUpSnooze = function() {
        if (localStorage.snoozedUntil > Date.now()) {
            snooze.textContent = text.get('unsnooze');
            snooze.onclick = function() {
                pb.unsnooze();

                setTimeout(function() {
                    setUpSnooze();
                }, 200);
            };
        } else {
            snooze.textContent = text.get('snooze');
            snooze.onclick = function() {
                pb.snooze();

                setTimeout(function() {
                    setUpSnooze();
                }, 200);
            };
        }
    };

    setUpSnooze();

    var options = document.getElementById('options');
    options.onclick = function() {
        if (window.chrome) {
            var optionsUrl = chrome.extension.getURL('options.html');

            chrome.tabs.query({ url: optionsUrl }, function(tabs) {
                if (tabs.length) {
                    chrome.tabs.update(tabs[0].id, { active: true });
                } else {
                    pb.openTab(optionsUrl);
                }

                menuSink.onclick();
            });
        } else if (window.safari) {
            pb.openTab(safari.extension.baseURI + 'options.html');
            safari.self.hide();
        } else {
            pb.openTab('about:addons');
        }

        menuSink.onclick();
    };


    document.getElementById('sign-out').onclick = function() {
        pb.signOut();
        if (window.chrome) {
            window.close();
        } else if (window.safari) {
            safari.self.hide();
        }
    };
};

var setUpTabs = function() {
    var peopleTab = document.getElementById('people-tab');
    var devicesTab = document.getElementById('devices-tab');
    var smsTab = document.getElementById('sms-tab');
    var notificationsTab = document.getElementById('notifications-tab');

    var tabs = [peopleTab, devicesTab, smsTab, notificationsTab];

    var onclick = function(e) {
        tabs.forEach(function(tab) {
            tab.classList.remove('selected');

            if (tab != devicesTab) {
                document.getElementById(tab.id + '-content').style.display = 'none';
            }
        });

        e.target.classList.add('selected');

        if (e.target == devicesTab) {
            document.getElementById(peopleTab.id + '-content').style.display = 'block';
        } else {
            document.getElementById(e.target.id + '-content').style.display = 'block';
        }

        localStorage.activePanelTab = e.target.id.split('-')[0];

        if (e.target == peopleTab || e.target == devicesTab) {
            setUpPeopleTab(e.target == peopleTab ? 'people': 'devices');

            setTimeout(function() {
                var lastTargetId;
                if (e.target == peopleTab) {
                    lastTargetId = localStorage.lastPeopleTargetId;
                } else {
                    lastTargetId = localStorage.lastDevicesTargetId;
                }

                var row = document.getElementById(lastTargetId);
                if (!row) {
                    row = document.getElementById('targets-bar').childNodes[0];
                }

                if (row) {
                    row.onclick();
                    if (row.id != '*') {
                        var  index = 0, element = row;
                        while ((element = element.previousElementSibling) != null) index++;

                        if (index > 6) {
                            row.scrollIntoView(true);
                        }
                    }
                }
            }, 100);

            document.getElementById('input').focus();
        }
    };

    tabs.forEach(function(tab) {
        tab.onclick = onclick;
    });

    if (localStorage.activePanelTab == 'notifications') {
        notificationsTab.click();
    } else if (localStorage.activePanelTab == 'sms') {
        smsTab.click();
    } else if (localStorage.activePanelTab == 'devices') {
        devicesTab.click();
    } else {
        peopleTab.click();
    }
};

var setUpPopout = function() {
    var popout = document.getElementById('popout-holder');
    popout.onclick = function() {
        pb.log('Popping out panel');

        pb.track({
            'name': 'panel_popped_out'
        });

        if (window.chrome) {
            var popoutUrl = chrome.extension.getURL('panel.html');

            chrome.tabs.query({ url: popoutUrl }, function(tabs) {
                if (tabs.length > 0) {
                    chrome.windows.update(tabs[0].windowId, { 'focused': true }, function() {
                        chrome.tabs.update(tabs[0].id, { 'active': true }, function() {
                        });
                    });
                } else {
                    chrome.windows.create({
                        'url': chrome.runtime.getURL('panel.html#popout'),
                        'type': 'popup',
                        'width': 640,
                        'height': 456,
                        'focused': true
                    });
                }

                window.close();
            });
        } else if (window.safari) {
        } else {
            self.port.emit('pop_out_panel');
        }
    };

    if (location.hash) {
        popout.style.display = 'none';
    }
};

var checkNativeClient = function() {
    utils.checkNativeClient(function(response) {
        if (response) {
            document.getElementById('snooze').style.display = 'none';
            document.getElementById('notifications-empty').classList.add('desktop-app');
            document.getElementById('notifications-empty-text').textContent = text.get('alert_desktop_app_notifications');
        }
    });
};

var monitorAwake = function() {
    pb.setAwake(location.hash ? location.hash.substring(1) : 'panel', true);

    document.body.onmousemove = function(e) {
        if (window.mouseLastX !== e.clientX || window.mouseLastY !== e.clientY) {
            reportAwake();
        }

        window.mouseLastX = e.clientX;
        window.mouseLastY = e.clientY;
    };
};

var lastReportedAwake = Date.now();
var reportAwake = function() {
    if (Date.now() - lastReportedAwake > 15 * 1000) {
        lastReportedAwake = Date.now();
        pb.setAwake(location.hash ? location.hash.substring(1) : 'panel', true);
    }
};

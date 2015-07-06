'use strict';

if (!self.port && !window.chrome && !window.safari) {
    throw new Error('Shouldn\'t be here');
}

onFocusChanged = function() {
    updateActiveChat();

    if (focused) {
        localsChangedListener();
    } else {
        if (window.safari) {
            onunload();
        }
    }
};

var updateActiveChat = function() {
    var target = document.getElementById('input').target;
    if (target && target.with && target.with.email_normalized) {
        pb.setActiveChat(tabId, {
            'email': target.with.email_normalized,
            'focused': focused
        });
    } else {
        pb.clearActiveChat(tabId);
    }
};

var mode;
var setUpPeopleTab = function(newMode) {
    mode = newMode;

    var input = document.getElementById('input');
    input.placeholder = location.hash ? text.get('message_placeholder_files') : text.get('message_placeholder');
    delete input.target;

    document.getElementById('chat-title').placeholder = text.get('title_placeholder');
    document.getElementById('chat-link').placeholder = text.get('url_placeholder');

    setUpInput();

    setUpDevicePicker();

    setUpLinkAttacher();

    setUpDropZone('chat-bar', function(file) {
        var target = realTarget();
        if (!target) {
            return;
        }

        var push = {
            'file': file
        };

        addTarget(target, push);

        pb.sendPush(push);
    });

    localsChangedListener();

    pb.addEventListener('locals_changed', localsChangedListener);
};

var tearDownPeopleTab = function() {
    pb.removeEventListener('locals_changed', localsChangedListener);
    pb.clearActiveChat(tabId);
};

var localsChangedListener = function(e) {
    if (!window) {
        return;
    }

    if (!pb.local.user) {
        return;
    }

    setUpTargetsBar();

    var target = document.getElementById('input').target;
    if (target) {
        document.getElementById(target.with ? target.with.email_normalized : target.iden).classList.add('selected');
        updateChatUi(target);
    }
};

var setUpInput = function() {
    var input = document.getElementById('input');

    input.addEventListener('input', function(e) {
        reportAwake();
    });

    var sendClicked = function() {
        input.focus();

        var target = realTarget();
        if (!target) {
            return;
        }

        var push =  {
            'type': 'note',
        };

        if (document.getElementById('chat-link-holder').style.display == 'block') {
            var linkTitle = document.getElementById('chat-title');
            var linkUrl = document.getElementById('chat-link');

            if (linkUrl.value) {
                push.type = 'link';
                push.url = linkUrl.value;
                push.tabId = linkUrl.tabId;
            }
            if (linkTitle.value) {
                push.title = linkTitle.value;
            }

            if (input.value) {
                push.body = input.value;
            }
        } else {
            if (input.value) {
                if (utils.isLink(input.value)) {
                    push.type = 'link';
                    push.url = input.value;
                } else {
                    push.body = input.value;
                }
            }
        }

        if (!push.body && !push.url && !push.title) {
            return;
        }

        addTarget(target, push);

        pb.sendPush(push);

        pb.setAwake('panel', true);

        input.value = '';

        document.getElementById('chat-link-close').onclick();

        scrollChat();

        reportAwake();
    };

    document.getElementById('chat-send-holder').onclick = function() {
        sendClicked();
    };

    input.onkeydown = function(e) {
        if (e.keyCode == utils.ENTER && !e.shiftKey) {
            sendClicked();
            return false;
        }
    };
};

var realTarget = function() {
    var target = document.getElementById('input').target;
    if (target) {
        if (target.iden == '*') {
            var device = document.getElementById('device').target;
            return device;
        } else {
            return target;
        }
    }
};

var addTarget = function(target, push) {
    if (target.with) {
        push.email = target.with.email_normalized;
    } else if (target.tag) {
        push.channel_tag = target.tag;
    } else if (target.iden == '*') {
    } else {
        push.device_iden = target.iden;
    }
};

var setUpDevicePicker = function() {
    document.getElementById('device').placeholder = text.get('devices_placeholder');

    var devices = utils.asArray(pb.local.devices).sort(function(a, b) {
        return b.created - a.created;
    });

    devices.unshift({
        'iden': '*',
        'image_url': 'chip_everything.png',
        'name': text.get('all_of_my_devices')
    });

    picker.setUp({
        'inputId': 'device',
        'pickerId': 'device-picker',
        'overlayId': 'device-overlay',
        'targets': devices
    });
};

var setUpTargetsBar = function() {
    var latestMap = { }, latestSelfPush;
    Object.keys(pb.local.pushes).forEach(function(iden) {
        var push = pb.local.pushes[iden];

        var keys = utils.targetKeys(push);
        keys.forEach(function(key) {
            var existing = latestMap[key];
            if (!existing || existing.created < push.created) {
                latestMap[key] = push;
            }
        });

        if (push.direction == 'self' && !push.client_iden) {
            if (!latestSelfPush || latestSelfPush.created < push.created) {
                latestSelfPush = push;
            }
        }
    });

    var targetRow = function(imageUrl, line1, line2, line2class, popOutClick) {
        var primary = document.createElement('div');
        primary.className = 'target-row-content-line';
        primary.textContent = line1;

        var content = document.createElement('div');
        content.className = 'target-row-content';
        content.appendChild(primary);

        if (line2 && line2 != '') {
            var secondary = document.createElement('div');
            secondary.className = 'target-row-content-line';
            secondary.textContent = line2;

            if (line2class) {
                secondary.classList.add(line2class);
            }

            content.appendChild(secondary);
        } else {
            primary.style.lineHeight = '36px';
        }

        var img = document.createElement('img');
        img.className = 'target-row-image';
        img.src = imageUrl;

        var div = document.createElement('div');
        div.className = 'target-row';
        div.appendChild(img);
        div.appendChild(content);

        if (popOutClick && !window.safari) {
            var popOutIcon = document.createElement('i');
            popOutIcon.className = 'pushfont-popout';

            var popOut = document.createElement('div');
            popOut.className = 'pop-out-target';
            popOut.appendChild(popOutIcon);
            popOut.onclick = popOutClick;

            div.appendChild(popOut);
        }

        return div;
    };

    var selectTarget = function(target) {
        var input = document.getElementById('input');
        input.target = target;
        input.value = '';
        input.focus();

        var id;
        if (target) {
            id = target.with ? target.with.email_normalized : target.iden;
        } else {
            id = '*';
        }

        if (!mode || mode == 'people') {
            localStorage.lastPeopleTargetId = id;
        } else {
            localStorage.lastDevicesTargetId = id;
        }

        var selectedSet = document.getElementsByClassName('target-row selected');
        for(var i = 0; i < selectedSet.length; i++) {
            var selected = selectedSet[i];
            selected.classList.remove('selected');
        }

        document.getElementById(id).classList.add('selected');

        if (!location.hash && (target.pushable || target.iden == '*')) {
            document.getElementById('chat-add-link').onclick();
        } else {
            document.getElementById('chat-link-close').onclick();
        }

        var devicePickerHolder = document.getElementById('device-picker-holder');
        var chatBarTop = document.getElementById('chat-bar-top');
        if (target.iden == '*') {
            devicePickerHolder.style.display = 'block';
            chatBarTop.classList.add('with-picker');
        } else {
            devicePickerHolder.style.display = 'none';
            chatBarTop.classList.remove('with-picker');
        }

        updateChatUi(target);

        scrollChat();

        updateActiveChat();
    };

    var meRow = function() {
        var imageUrl = pb.local.user.image_url || 'chip_person.png';
        var line1 = text.get('me');
        var line2 = latestSelfPush ? latestSelfPush.title || latestSelfPush.body || latestSelfPush.url || latestSelfPush.file_name : '';
        var row = targetRow(imageUrl, line1, line2, 'secondary');
        row.id = '*';
        row.onclick = function() {
            selectTarget({ 'iden': row.id });
        };

        return row;
    };

    var newChatRow = function() {
        var imageUrl = 'chip_add.png';
        var line1 = text.get('add_a_friend');
        var row = targetRow(imageUrl, line1, '', 'secondary');
        row.onclick = function() {
            pb.openTab(pb.www + '/#people/new');
        };

        return row;
    };

    var line2 = function(push) {
        if (!push) {
            return '';
        }

        return push.title || push.body || push.url || push.file_name || '';
    };

    var fragment = document.createDocumentFragment();

    if (!mode || mode == 'people') {
        fragment.appendChild(meRow());
    }

    var targets = (!mode || mode == 'people') ? utils.asArray(pb.local.chats) : utils.asArray(pb.local.devices);

    var latestKey = function(target) {
        return target.with ? target.with.email_normalized : target.client ? target.client.iden : target.channel ? target.channel.iden : target.iden;
    };

    targets.sort(function(a, b) {
        var al = latestMap[latestKey(a)];
        var bl = latestMap[latestKey(b)];
        if (al && !bl) {
            return -1;
        } else if (!al && bl) {
            return 1;
        } else if (al && bl) {
            return bl.created - al.created;
        } else {
            var an, bn;
            if (a.with) {
                an = a.with.name ? a.with.name.toLowerCase() : a.with.email_normalized;
            } else {
                an = (a.name || a.nickname || a.model || a.tag).toLowerCase();
            }
            if (b.with) {
                bn = b.with.name ? b.with.name.toLowerCase() : b.with.email_normalized;
            } else {
                bn = (b.name || b.nickname || b.model || b.tag).toLowerCase();
            }

            if (an > bn) {
                return 1;
            } else if (an < bn) {
                return -1;
            } else {
                return 0;
            }
        }
    });

    targets.forEach(function(target) {
        var latest = latestMap[latestKey(target)];

        var line2class = 'secondary';
        if (latest&& (target.tag || target.with)) {
            if (latest.direction == 'incoming') {
                if (!latest.dismissed) {
                    line2class = 'new-message';
                } else if (latest.awake_app_guids && latest.awake_app_guids.indexOf('extension-' + localStorage.client_id) != -1) {
                    line2class = 'new-message';
                }
            }
        }

        var popOutClick;
        if (!!target.with) {
            popOutClick = function(e) {
                e.stopPropagation();
                pb.openChat(target.with.email_normalized);
            };
        }

        var row = targetRow(utils.targetImageUrl(target),
                            utils.targetDisplayName(target),
                            line2(latest),
                            line2class,
                            popOutClick);

        row.id = target.with ? target.with.email_normalized : target.iden;
        row.onclick = function() {
            selectTarget(target);
        };

        fragment.appendChild(row);
    });

    if (!mode || mode == 'people') {
        fragment.appendChild(newChatRow());
    }

    var targetsBar = document.getElementById('targets-bar');

    while (targetsBar.hasChildNodes()) {
        targetsBar.removeChild(targetsBar.lastChild);
    }

    targetsBar.appendChild(fragment);
};

var setUpLinkAttacher = function() {
    var input = document.getElementById('input');
    var addLink = document.getElementById('chat-add-link');
    var addLinkPoker = document.getElementById('chat-link-holder-poker');
    var removeLink = document.getElementById('chat-link-close');
    var addLinkTooltop = document.getElementById('chat-add-link-tooltip');
    var chatBarTop = document.getElementById('chat-bar-top');
    var sendHolder = document.getElementById('chat-send-holder');
    var inputHolder = document.getElementById('chat-input-holder');
    var linkHolder = document.getElementById('chat-link-holder');
    var devicePicker = document.getElementById('device-picker');
    var linkTitle = document.getElementById('chat-title');
    var linkUrl = document.getElementById('chat-link');

    addLink.onmouseenter = function() {
        addLinkTooltop.style.display = 'block';
    };

    addLink.onmouseleave = function() {
        addLinkTooltop.style.display = 'none';
    };

    var attach = function() {
        pb.getActiveTab(function(tab) {
            if (!tab  || !tab.url || tab.url.indexOf('http') != 0) {
                removeLink.onclick();
                return;
            }

            addLink.onclick = remove;

            input.placeholder = text.get('message_placeholder_link');
            linkHolder.style.display = 'block';
            addLinkPoker.style.display = 'block';
            chatBarTop.classList.add('with-link');
            sendHolder.classList.add('with-link');
            inputHolder.classList.add('with-link');
            devicePicker.classList.add('with-link');
            addLink.classList.add('with-link');
            addLinkTooltop.textContent = text.get('remove_link_tooltip');

            linkTitle.value = tab.title || '';
            linkUrl.value = tab.url || '';
            linkUrl.tabId = tab.id;
            document.getElementById('chat-link-favicon').src = tab.favIconUrl || 'link.png';

            scrollChat();
        });
    };

    var remove = function() {
        linkHolder.style.display = 'none';

        addLink.onclick = attach;

        input.placeholder = location.hash ? text.get('message_placeholder_files') : text.get('message_placeholder');
        chatBarTop.classList.remove('with-link');
        sendHolder.classList.remove('with-link');
        devicePicker.classList.remove('with-link');
        addLink.classList.remove('with-link');
        addLinkPoker.style.display = 'none';
        addLinkTooltop.textContent = text.get('add_link_tooltip');

        if (location.hash) {
            inputHolder.classList.add('with-link');
            addLink.style.display = 'none';
        } else {
            inputHolder.classList.remove('with-link');
            addLink.style.display = 'block';
        }

        document.getElementById('chat-bar-top').classList.remove('with-link');
    };

    addLink.onclick = attach;
    removeLink.onclick = remove;
};

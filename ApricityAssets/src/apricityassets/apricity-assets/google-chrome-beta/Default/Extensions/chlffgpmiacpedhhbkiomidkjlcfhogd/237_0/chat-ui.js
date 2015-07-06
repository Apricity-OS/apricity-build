'use strict';

window.onresize = function() {
    updateMaxWidth();
};

var updateMaxWidth = function() {
    var chatHolder = document.getElementById('chat-holder');
    var maxWidth = chatHolder.clientWidth - 72;

    for (var i = 0; i < document.styleSheets.length; i++) {
        var styleSheet = document.styleSheets[i];
        if (styleSheet.href && styleSheet.href.indexOf('chat-ui.css') != -1) {
            for (var j = 0; j < styleSheet.cssRules.length; j++) {
                var rule = styleSheet.cssRules[j];
                if (rule.selectorText == '.chat-bubble') {
                    rule.style.maxWidth = maxWidth + 'px';
                    return;
                }
            }
        }
    }
};

var initialResize = false;
var drawChat = function(pushes, targets, retryFailed, clearFailed, cancelUpload, historyLink) {
    if (!initialResize) {
        updateMaxWidth();
    }

    var chat = document.getElementById('chat');
    var chatHolder = document.getElementById('chat-holder');
    var atBottom = chatHolder.scrollTop == 0 || chatHolder.scrollTop + chatHolder.offsetHeight >= chatHolder.scrollHeight;

    while (chat.hasChildNodes()) {
        chat.removeChild(chat.lastChild);
    }

    chat.appendChild(seeFullHistory(historyLink));

    if (!pushes || pushes.length == 0) {
        chat.appendChild(emptyState());
        return;
    }

    var chunks = chunkify(pushes);
    chat.appendChild(renderChunks(chunks, targets));

    if (atBottom) {
        scrollChat();
    }
};

var scrollChat = function() {
    var chatHolder = document.getElementById('chat-holder');
    chatHolder.scrollTop = chatHolder.scrollHeight;
};

var seeFullHistory = function(historyLink) {
    var a = document.createElement('a');
    a.id = 'chat-see-history';
    a.target = '_blank';
    a.href = historyLink;
    a.textContent = text.get('see_full_history');

    var div = document.createElement('div');
    div.id = 'chat-see-history-holder';
    div.appendChild(a);

    return div;
};

var emptyState = function() {
    var img = document.createElement('img');
    img.src = 'bg_sam.png';

    var p = document.createElement('p');
    p.textContent = text.get('no_pushes');

    var div = document.createElement('div');
    div.id = 'chat_empty_state';
    div.appendChild(img);
    div.appendChild(p);

    return div;
};

var chunkify = function(pushes) {
    var chunks = [], chunk = [], previous;

    var nextChunk = function(push) {
        chunks.push(chunk);
        chunk = [];
        chunk.push(push);
    };

    pushes.forEach(function(push) {
        if (!previous) {
            chunk.push(push);
        } else if (push.direction == 'self') {
            if (!push.client_iden) {
                var hasSource = !!push.source_device_iden;
                var hasTarget = !!push.target_device_iden;
                var previousHasSource = !!previous.source_device_iden;
                var previousHasTarget = !!previous.target_device_iden;
                var sourcesMatch = push.source_device_iden == previous.source_device_iden;
                var targetsMatch = push.target_device_iden == previous.target_device_iden;

                if (previous.client_iden) {
                    nextChunk(push);
                } else if (!hasSource && !previousHasSource && !hasTarget && !previousHasTarget) {
                    chunk.push(push);
                } else if (hasSource && !hasTarget && !previousHasTarget && sourcesMatch) {
                    chunk.push(push);
                } else if (hasTarget && !hasSource && !previousHasSource && targetsMatch) {
                    chunk.push(push);
                } else if (hasSource && sourcesMatch && hasTarget && targetsMatch) {
                    chunk.push(push);
                } else {
                    nextChunk(push);
                }
            } else {
                if (push.client_iden == previous.client_iden) {
                    chunk.push(push);
                } else {
                    nextChunk(push);
                }
            }
        } else if (push.direction == 'incoming') {
            if (push.channel_iden && push.channel_iden == previous.channel_iden) {
                chunk.push(push);
            } else if (push.sender_email_normalized && push.sender_email_normalized == previous.sender_email_normalized) {
                chunk.push(push);
            } else {
                nextChunk(push);
            }
        } else if (push.direction == 'outgoing') {
            if (push.receiver_email_normalized && push.receiver_email_normalized == previous.receiver_email_normalized) {
                chunk.push(push);
            } else {
                nextChunk(push);
            }
        } else {
            if (push.email == previous.email) {
                chunk.push(push);
            } else if (push.email == previous.receiver_email_normalized && previous.direction != 'incoming') {
                chunk.push(push);
            } else if (push.device_iden && push.device_iden == previous.device_iden) {
                chunk.push(push);
            } else if (push.device_iden && push.device_iden == previous.target_device_iden) {
                chunk.push(push);
            } else {
                nextChunk(push);
            }
        }

        previous = push;
    });

    chunks.push(chunk);

    return chunks;
};

var loadedImages = {};
var renderChunks = function(chunks, targets) {
    var fragment = document.createDocumentFragment();
    var chatHolder = document.getElementById('chat-holder');

    var previousPush;
    chunks.forEach(function(chunk) {
        var chunkHolder = document.createElement('div');
        chunkHolder.className = 'chunk-holder';

        var lastChunk = chunks[chunks.length - 1];
        var lastPush = chunk[chunk.length - 1];
        chunk.forEach(function(push) {
            if (!previousPush || push.created - previousPush.created > 15 * 60) {
                chunkHolder.appendChild(timeDivider(push));
            }

            var onLeft = isOnLeft(push, targets);

            var row = document.createElement('div');
            row.className = 'chat-row';

            var contents = document.createElement('div');
            contents.className = 'chat-bubble-contents';

            if (push.title) {
                var p = document.createElement('p');
                p.className = 'push-title';
                p.textContent = push.title;
                contents.appendChild(p);
            }

            if (push.body) {
                var p = document.createElement('p');
                p.className = 'push-body';

                utils.linkify(push.body, p);

                contents.appendChild(p);
            }

            var imgHolder = document.createElement('div');
            contents.appendChild(imgHolder);

            if (push.image_url || (push.file && ['image/gif', 'image/png', 'image/jpg', 'image/jpeg'].indexOf(push.file.type) != -1)) {
                var maxWidth = 192;

                var img = document.createElement('img');
                img.className= 'push-image';
                img.style.maxWidth = maxWidth + 'px';

                if (push.file) {
                    if (!push.imgElement) {
                        var reader = new FileReader();
                        reader.readAsDataURL(push.file);
                        reader.onload = function() {
                            push.dataUrl = reader.result;
                            img.src = push.dataUrl;
                            push.imgElement = img;
                        };
                    } else {
                        img = push.imgElement;
                    }
                } else {
                    var resizeTo = maxWidth;
                    if (push.image_width && push.image_height) {
                        if (push.image_width > maxWidth) {
                            var factor = maxWidth / push.image_width;
                            var scaledWidth = Math.round(factor * push.image_width);
                            var scaledHeight = Math.round(factor * push.image_height);
                            resizeTo = Math.max(scaledWidth, scaledHeight);

                            img.style.width = scaledWidth + 'px';
                            img.style.height = scaledHeight + 'px';
                        } else {
                            img.style.width = push.image_width + 'px';
                            img.style.height = push.image_height + 'px';
                        }
                    }

                    if (push.image_url.indexOf('imgix') != -1) {
                        img.src = push.image_url + '?w=' + maxWidth + '&fit=max';
                    } else if (push.image_url.indexOf('ggpht') != -1 || push.image_url.indexOf('googleusercontent')) {
                        img.src = push.image_url + '=s' + resizeTo;
                    } else {
                        img.src = push.image_url;
                    }

                    img.onclick = function() {
                        window.open(push.file_url);
                    };
                }

                if (!loadedImages[img.src]) {
                    img.onload = function() {
                        loadedImages[img.src] = true;

                        if (chatHolder.scrollTop != 0) {
                            chatHolder.scrollTop += contents.offsetHeight;
                        }
                    };
                }

                img.onerror = function() {
                    img.style.display = 'none';
                };

                imgHolder.appendChild(img);
            } else {
                var url = push.url || push.file_url;
                if (url) {
                    var a = document.createElement('a');
                    a.className = 'push-url';
                    a.href = url;
                    a.target = '_blank';
                    a.textContent = push.file_name || url;

                    if (onLeft) {
                        a.classList.add('left');
                    }

                    contents.appendChild(a);
                } else if (push.file) {
                    var p = document.createElement('p');
                    p.textContent = push.file.name;

                    contents.appendChild(p);
                }
            }

            if (push.progress && push.progress < 1) {
                var progressBar = document.createElement('div');
                progressBar.className = 'chat-progress-bar';

                var progressBarFill = document.createElement('div');
                progressBarFill.className = 'chat-progress-bar-fill';
                progressBarFill.style.width = (push.progress * 100) + '%';

                progressBar.appendChild(progressBarFill);

                contents.appendChild(progressBar);
            }

            var bubble = document.createElement('div');
            bubble.className = 'chat-bubble';
            bubble.appendChild(contents);

            bubble.setAttribute('data-push-iden', push.iden);

            if (onLeft) {
                bubble.classList.add('left');
            }

            if (push.failed) {
                bubble.classList.add('failed');

                var errorMessage;
                if (push.error && push.error.message.indexOf('too big') != -1) {
                    errorMessage = document.createElement('a');
                    errorMessage.className = 'sadface';
                    errorMessage.textContent = text.get('file_too_big');
                    errorMessage.onclick = function() {
                        pb.openTab('https://help.pushbullet.com/articles/is-there-a-file-size-limit/');
                    };
                } else {
                    errorMessage = document.createElement('span');
                    errorMessage.className = 'sadface';
                    errorMessage.textContent = text.get('send_failed');
                }

                var retry = document.createElement('a');
                retry.className = 'fail-button';
                retry.textContent = text.get('retry');
                retry.onclick = function() {
                    retryFailed(push);
                };

                var clear = document.createElement('a');
                clear.className = 'fail-button';
                clear.textContent = text.get('clear');
                clear.onclick = function() {
                    clearFailed(push);
                };

                var failed = document.createElement('div');
                failed.appendChild(errorMessage);
                failed.appendChild(retry);
                failed.appendChild(clear);

                contents.appendChild(failed);
            } else if (push.queued) {
                // row.classList.add('queued');
            }

            row.appendChild(bubble);

            if (push.file) {
                var cancel = document.createElement('i');
                cancel.className = 'cancel pushfont-close';
                cancel.onclick = function() {
                    cancelUpload(push);
                };

                row.appendChild(cancel);
            }

            if (push == lastPush) {
                var thumb = document.createElement('img');
                thumb.className = 'chat-image';
                thumb.src = utils.targetImageUrl(findTarget(push, targets));

                if (onLeft) {
                    thumb.classList.add('left');
                }

                row.appendChild(thumb);

                var poker = document.createElement('div');
                poker.className = 'chat-poker';

                if (onLeft) {
                    poker.classList.add('left');
                    poker.innerHTML = '<svg><polygon points="12,0 0,10 12,10"></svg>';
                } else {
                    poker.innerHTML = '<svg><polygon points="0,0 12,10 0,10"></svg>';
                }

                if (push.failed) {
                    poker.classList.add('failed');
                }

                row.appendChild(poker);
            }

            chunkHolder.appendChild(row);

            previousPush = push;
        });

        if (chunk == lastChunk && previousPush != null) {
            var div = document.createElement('div');
            div.className = 'push-date';

            if (previousPush.queued && !previousPush.failed) {
                div.textContent = text.get('sending');
            } else {
                var created = momentFrom(previousPush);
                if (Date.now() - created > 5 * 60 * 1000) {
                    var dateText;
                    if (Date.now() - created > 24 * 60 * 60 * 1000) {
                        dateText = created.format('ll');
                    } else {
                        dateText = created.fromNow();
                    }

                    div.textContent = dateText;
                } else {
                    div.textContent = text.get('now');
                }
            }

            if (isOnLeft(previousPush, targets)) {
                div.style.marginLeft = '50px';
            } else {
                div.style.textAlign = 'right';
                div.style.marginRight = '50px';
            }

            chunkHolder.appendChild(div);
        }

        fragment.appendChild(chunkHolder);
    });

    return fragment;
};

var isOnLeft = function(push, targets) {
    return push.direction == 'incoming'
            || push.client_iden
            || (push.direction == 'self' && targets.device && push.source_device_iden != targets.device.iden);
};

var momentFrom = function(push) {
    return moment(Math.floor(push.created ? Math.min(push.created * 1000, Date.now()) : Date.now()));
};

var timeDivider = function(push) {
    var created = momentFrom(push);

    var divider = document.createElement('div');
    divider.className = 'time-divider';
    divider.textContent = created.calendar();

    return divider;
};

var findTarget = function(push, targets) {
    var target;
    if (push.direction == 'incoming') {
        if (push.channel_iden) {
            targets.subscriptions.forEach(function(subscription) {
                if (push.channel_iden == subscription.channel.iden) {
                    target = subscription;
                }
            });

            targets.channels.forEach(function(channel) {
                if (push.channel_iden == channel.iden) {
                    target = channel;
                }
            });
        } else {
            targets.chats.forEach(function(chat) {
                if (push.sender_email_normalized == chat.with.email_normalized) {
                    target = chat;
                }
            });
        }
    } else if (push.direction == 'outgoing') {
        return targets.me;
    } else {
        if (push.email) {
            return targets.me;
        } else if (push.channel_tag) {
            return targets.me;
        } else {
            targets.devices.forEach(function(device) {
                if (push.source_device_iden == device.iden) {
                    target = device;
                }
            });

            if (!target) {
                return targets.me;
            }
        }
    }

    return target;
};

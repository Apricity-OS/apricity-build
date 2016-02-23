#! /bin/bash
gsettings set org.gnome.settings-daemon.peripherals.touchpad tap-to-click true
gsettings set org.gnome.desktop.interface gtk-theme "Arctic Apricity"
gsettings set org.gnome.desktop.wm.preferences theme "Arctic Apricity"
gsettings set org.gnome.shell.extensions.user-theme name "Arctic Apricity"
gsettings set org.gnome.nautilus.icon-view default-zoom-level 'small'
gsettings set org.gnome.desktop.interface icon-theme "Apricity Icons"
gsettings set org.gnome.shell enabled-extensions "['user-theme@gnome-shell-extensions.gcampax.github.com', 'alwayszoomworkspaces@jamie.thenicols.net', 'mediaplayer@patapon.info', 'caffeine@patapon.info', 'scroll-workspaces@gfxmonk.net', 'dash-to-dock@micxgx.gmail.com', 'suspend-button@laserb', 'topIcons@adel.gadllah@gmail.com', 'places-menu@gnome-shell-extensions.gcampax.github.com', 'drive-menu@gnome-shell-extensions.gcampax.github.com', 'remove-dropdown-arrows@mpdeimos.com', 'Move_Clock@rmy.pobox.com']"
gsettings set org.gnome.desktop.wm.preferences button-layout ':minimize,maximize,close'
gsettings set org.gnome.settings-daemon.plugins.xsettings overrides "{'Gtk/DecorationLayout': <':minimize,maximize,close'>}"
gsettings set org.gnome.shell.overrides dynamic-workspaces false
gsettings set org.gnome.desktop.background show-desktop-icons true
gconftool-2 --set /desktop/gnome/background/picture_filename --type=string "/usr/share/backgrounds/gnome/ripples.jpg"
gsettings set org.gnome.shell favorite-apps "['google-chrome.desktop', 'ice.desktop', 'org.gnome.Nautilus.desktop', 'geary.desktop', 'empathy.desktop', 'gnome-music.desktop', 'org.gnome.Photos.desktop', 'org.gnome.Totem.desktop', 'libreoffice-writer.desktop', 'libreoffice-impress.desktop', 'libreoffice-calc.desktop', 'org.gnome.gedit.desktop', 'gnome-terminal.desktop', 'gnome-tweak-tool.desktop', 'gnome-control-center.desktop', 'pamac-manager.desktop', 'cnchi.desktop']"
gsettings set org.freedesktop.Tracker.Miner.Files index-recursive-directories "['$HOME', '&DOCUMENTS', '&DOWNLOAD', '&MUSIC', '&PICTURES', '&VIDEOS']"
gsettings set org.freedesktop.Tracker.Miner.Files crawling-interval -2
gsettings set org.freedesktop.Tracker.Miner.Files enable-monitors false
tracker-control -r
gsettings set org.gnome.desktop.background picture-uri file:///usr/share/backgrounds/gnome/tahoe.jpg
gsettings set org.gnome.desktop.screensaver picture-uri file:///usr/share/backgrounds/gnome/bliss.jpg

gnome-shell --replace

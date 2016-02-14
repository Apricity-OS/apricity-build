#! /bin/bash
gsettings set org.gnome.settings-daemon.peripherals.touchpad tap-to-click true
gsettings set org.gnome.desktop.interface gtk-theme "Arctic Apricity"
gsettings set org.gnome.desktop.wm.preferences theme "Arctic Apricity"
gsettings set org.cinnamon.theme name "Arctic Apricity"
gsettings set org.gnome.desktop.interface icon-theme "Apricity Icons"
gsettings set org.gnome.desktop.wm.preferences button-layout ':minimize,maximize,close'
gsettings set org.gnome.settings-daemon.plugins.xsettings overrides "{'Gtk/DecorationLayout': <':minimize,maximize,close'>}"
gsettings set org.cinnamon favorite-apps "['google-chrome.desktop', 'ice.desktop', 'org.gnome.Nautilus.desktop', 'geary.desktop', 'empathy.desktop', 'gnome-music.desktop', 'org.gnome.Photos.desktop', 'org.gnome.Totem.desktop', 'libreoffice-writer.desktop', 'libreoffice-impress.desktop', 'libreoffice-calc.desktop', 'org.gnome.gedit.desktop', 'gnome-terminal.desktop', 'gnome-tweak-tool.desktop', 'gnome-control-center.desktop', 'pamac-manager.desktop', 'calamares.desktop']"
gsettings set org.freedesktop.Tracker.Miner.Files index-recursive-directories "['$HOME', '&DOCUMENTS', '&DOWNLOAD', '&MUSIC', '&PICTURES', '&VIDEOS']"
gsettings set org.freedesktop.Tracker.Miner.Files crawling-interval -2
gsettings set org.freedesktop.Tracker.Miner.Files enable-monitors false
tracker-control -r
gsettings set org.cinnamon.desktop.background picture-uri file:///usr/share/backgrounds/gnome/tahoe.jpg
# gsettings set org.gnome.desktop.screensaver picture-uri file:///usr/share/backgrounds/gnome/bliss.jpg

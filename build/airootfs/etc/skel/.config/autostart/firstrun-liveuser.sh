#! /bin/bash
if [ -e ~/.firstrun.ran ]
then
	cd /home

else
cp ~/.liquidprompt/liquidpromptrc-dist ~/.config/liquidpromptrc

echo 'source ~/.liquidprompt/liquidprompt' >> ~/.bashrc

sleep .15
gsettings set org.gnome.settings-daemon.peripherals.touchpad tap-to-click true
gsettings set org.gnome.desktop.interface gtk-theme "Arctic Apricity"
gsettings set org.gnome.desktop.wm.preferences theme "Arctic Apricity"
gsettings set org.gnome.shell.extensions.user-theme name "Arctic Apricity"
gsettings set org.gnome.desktop.interface icon-theme "Apricity Icons"

gsettings set org.gnome.shell enabled-extensions "['user-theme@gnome-shell-extensions.gcampax.github.com', 'alwayszoomworkspaces@jamie.thenicols.net', 'mediaplayer@patapon.info', 'caffeine@patapon.info', 'scroll-workspaces@gfxmonk.net', 'dash-to-dock@micxgx.gmail.com', 'shellshape@gfxmonk.net', 'suspend-button@laserb', 'topIcons@adel.gadllah@gmail.com', 'places-menu@gnome-shell-extensions.gcampax.github.com', 'drive-menu@gnome-shell-extensions.gcampax.github.com', 'remove-dropdown-arrows@mpdeimos.com']"

gsettings set org.gnome.desktop.wm.preferences button-layout ':minimize,maximize,close'
gsettings set org.gnome.settings-daemon.plugins.xsettings overrides "{'Gtk/DecorationLayout': <':minimize,maximize,close'>}"

gsettings set org.gnome.shell.overrides dynamic-workspaces false
gsettings set org.gnome.desktop.background show-desktop-icons true
gsettings set org.gnome.shell favorite-apps "['google-chrome-beta.desktop', 'firefox.desktop', 'org.gnome.Nautilus.desktop', 'geary.desktop', 'empathy.desktop', 'gnome-music.desktop', 'org.gnome.Photos.desktop', 'org.gnome.Totem.desktop', 'libreoffice-writer.desktop', 'libreoffice-impress.desktop', 'libreoffice-calc.desktop', 'org.gnome.gedit.desktop', 'gnome-terminal.desktop', 'gnome-tweak-tool.desktop', 'gnome-control-center.desktop', 'argon.desktop', 'cnchi.desktop']"
gsettings set org.freedesktop.Tracker.Miner.Files index-recursive-directories "[]"
gsettings set org.freedesktop.Tracker.Miner.Files crawling-interval -2
gsettings set org.freedesktop.Tracker.Miner.Files enable-monitors false
tracker-control -r


sudo sed -i 's@/usr/share/argon/argon.png@gnome-app-install@' /usr/share/applications/argon.desktop
sudo sed -i 's@/usr/share/argon/argon.png@gnome-app-install@' /usr/share/applications/argon-notifier-config.desktop
sudo sed -i 's@Icon=gnome-books@Icon=unity-webapps-amazoncloudreader@' /usr/share/applications/org.gnome.Books.desktop
sudo sed -i 's@Icon=builder@Icon=textwrangler@' /usr/share/applications/org.gnome.Builder.desktop
sudo sed -i 's@Icon=gnome-characters@Icon=accessories-character-map@' /usr/share/applications/org.gnome.Characters.desktop
sudo sed -i 's@Icon=x-office-address-book@Icon=evolution-addressbook@' /usr/share/applications/org.gnome.Contacts.desktop
sudo sed -i 's@Icon=grsync.png@Icon=luckybackup@' /usr/share/applications/grsync.desktop
sudo sed -i 's@Icon=xterm-color_48x48@Icon=xorg@' /usr/share/applications/xterm.desktop
sudo sed -i 's@Icon=tracker@Icon=preferences-system-search@' /usr/share/applications/tracker-preferences.desktop

#sudo sed -i 's@Icon=/usr/share/playonlinux/etc/playonlinux.png@Icon=playonlinux@' /usr/share/applications/playonlinux.desktop

sudo cp -f /etc/apricity-assets/playonlinux.png /usr/share/playonlinux/etc
sudo cp -f /etc/apricity-assets/playonlinux15.png /usr/share/playonlinux/etc
sudo cp -f /etc/apricity-assets/playonlinux16.png /usr/share/playonlinux/etc
sudo cp -f /etc/apricity-assets/playonlinux22.png /usr/share/playonlinux/etc
sudo cp -f /etc/apricity-assets/playonlinux32.png /usr/share/playonlinux/etc
#cp /etc/apricity-assets/chrome-apps/*.desktop /home/alex/.local/share/applications

sudo rm /usr/share/gnome-background-properties/adwaita.xml
sudo rm -r /usr/share/backgrounds/gnome
sudo mv /usr/share/backgrounds/apricity /usr/share/backgrounds/gnome
sudo mv -f /usr/share/gnome-background-properties/apricity-backgrounds.xml /usr/share/gnome-background-properties/gnome-backgrounds.xml
gsettings set org.gnome.desktop.background picture-uri file:///usr/share/backgrounds/gnome/flow.jpg
gsettings set org.gnome.desktop.screensaver picture-uri file:///usr/share/backgrounds/gnome/aurora.jpg
#cp -rf /etc/apricity-theme/google-chrome ~/.config
sudo rm /usr/share/applications/bssh.desktop
sudo rm /usr/share/applications/bvnc.desktop
sudo rm /usr/share/applications/avahi-discover.desktop
sudo rm /usr/share/applications/qv4l2.desktop
sudo rm /usr/share/applications/polkit-gnome-authentication-agent-1.desktop
sudo rm /usr/share/applications/tracker-needle.desktop
sudo rm /usr/share/applications/zenmap.desktop
sudo rm /usr/share/applications/zenmap-root.desktop
sudo rm /usr/share/applications/gksu.desktop
sudo rm /usr/share/applications/gucharmap.desktop
sudo rm /usr/share/applications/cups.desktop
#sudo rm /usr/share/applications/xterm.desktop
sudo rm /usr/share/applications/uxterm.desktop
sudo rm /usr/share/applications/epiphany.desktop
sudo rm /usr/share/applications/empathy.desktop
sudo rm /usr/share/applications/designer-qt4.desktop
sudo rm /usr/share/applications/linguist-qt4.desktop
sudo rm /usr/share/applications/assistant-qt4.desktop
sudo rm /usr/share/applications/qdbusviewer-qt4.desktop
rm ~/.local/share/applications/chrome-app-list.desktop

gsettings set org.freedesktop.Tracker.Miner.Files index-recursive-directories "[]"
gsettings set org.freedesktop.Tracker.Miner.Files crawling-interval -2
gsettings set org.freedesktop.Tracker.Miner.Files enable-monitors false
tracker-control -r

cp -rf /etc/apricity-assets/google-chrome-beta ~/.config/
#pipelight-plugin --accept --enable silverlight
#rm ~/.wine-pipelight
#sudo pipelight-plugin --create-mozilla-plugins

#sudo rm -r /usr/lib/evolution-data-server
#killall evolution-addressbook-factory
#killall evolution-addressbook-factory-subprocess
#killall evolution-calendar-factory
#killall evolution-calendar-factory-subprocess
#killall evolution-source-registry
killall tracker-store
killall tracker-extract
killall tracker-miner-apps
killall tracker-miner-fs
killall tracker-miner-user-guides

sudo pacman -Syy
sleep 2
gnome-shell --replace
touch ~/.firstrun.ran

fi

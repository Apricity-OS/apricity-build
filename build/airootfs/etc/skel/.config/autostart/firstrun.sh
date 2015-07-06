#! /bin/bash
if [ -e ~/.firstrun.ran ]
then
	cd /home

else
cp ~/.liquidprompt/liquidpromptrc-dist ~/.config/liquidpromptrc

echo 'source ~/.liquidprompt/liquidprompt' >> ~/.bashrc
gsettings set org.gnome.settings-daemon.peripherals.touchpad tap-to-click true
gsettings set org.gnome.desktop.interface gtk-theme "Arctic Apricity"
gsettings set org.gnome.desktop.wm.preferences theme "Arctic Apricity"
#gsettings set org.gnome.shell.extensions.user-theme name "Apricity OS 2"
gsettings set org.gnome.desktop.interface icon-theme "Apricity Icons"

gsettings set org.gnome.shell enabled-extensions "['user-theme@gnome-shell-extensions.gcampax.github.com', 'dash-to-dock@micxgx.gmail.com', 'mediaplayer@patapon.info', 'systemMonitor@gnome-shell-extensions.gcampax.github.com', 'caffeine@patapon.info', 'hide-dash@zacbarton.com', 'message_tray_on_bottom_right_corner@zzat', 'shellshape@gfxmonk.net', 'apps-menu@gnome-shell-extensions.gcampax.github.com']"

dconf write /org/gnome/shell/overrides/button-layout '":minimize,maximize,close"'
gsettings set org.gnome.settings-daemon.plugins.xsettings overrides "{'Gtk/DecorationLayout': <':minimize,maximize,close'>}"

gsettings set org.gnome.desktop.background picture-uri file:///usr/share/backgrounds/gnome/Whispy_Tails.jpg

gsettings set org.freedesktop.Tracker.Miner.Files index-recursive-directories "[]"
gsettings set org.freedesktop.Tracker.Miner.Files crawling-interval -2
gsettings set org.freedesktop.Tracker.Miner.Files enable-monitors false
tracker-control -r

#cp -rf /etc/apricity-theme/google-chrome ~/.config

touch ~/.firstrun.ran

fi

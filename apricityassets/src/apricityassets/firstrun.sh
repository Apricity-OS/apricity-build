#! /bin/bash
if [ -e ~/.firstrun.ran ]
then
	cd /home

else

echo 'source ~/.liquidprompt/liquidprompt' >> ~/.bashrc
touch ~/.firstrun.ran
/etc/skel/.config/autostart/firstrun-desktop.sh
rm ~/.local/share/applications/chrome-app-list.desktop
cp -rf /etc/apricity-assets/google-chrome ~/.config/
cp ~/.liquidprompt/liquidpromptrc-dist ~/.config/liquidpromptrc
#su -c "sed -i /etc/pam.d/su -e 's/.*wheel/#&/'"
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
sleep 2

fi

#!/bin/bash

#Archiso Stuff
set -e -u
umask 022
sed -i 's/#\(en_US\.UTF-8\)/\1/' /etc/locale.gen
locale-gen
ln -sf /usr/share/zoneinfo/UTC /etc/localtime
usermod -s /usr/bin/zsh root
cp -aT /etc/skel/ /root/
chmod 700 /root

#Create Liveuser
id -u liveuser &>/dev/null || useradd -m "liveuser" -g users -G "adm,audio,floppy,log,network,rfkill,scanner,storage,optical,power,wheel"
passwd -d liveuser
echo 'Created User'

#Name Apricity
sed -i.bak 's/Arch Linux/Apricity OS/g' /usr/lib/os-release
sed -i.bak 's/arch/apricity/g' /usr/lib/os-release
sed -i.bak 's/www.archlinux.org/www.apricityos.com/g' /usr/lib/os-release
sed -i.bak 's/bbs.archlinux.org/www.apricityos.com/g' /usr/lib/os-release
sed -i.bak 's/bugs.archlinux.org/www.apricityos.com/g' /usr/lib/os-release
cp /usr/lib/os-release /etc/os-release

#Run Architecture-Specific Tasks
arch=`uname -m`
if [ "$arch" == "x86_64" ]
then
# Remove Unused Desktop Applications
	rm -f /usr/share/applications/bssh.desktop
	rm -f /usr/share/applications/bvnc.desktop
	rm -f /usr/share/applications/avahi-discover.desktop
	rm -f /usr/share/applications/qv4l2.desktop
	rm -f /usr/share/applications/polkit-gnome-authentication-agent-1.desktop
	rm -f /usr/share/applications/tracker-needle.desktop
	rm -f /usr/share/applications/gksu.desktop
	rm -f /usr/share/applications/gucharmap.desktop
	rm -f /usr/share/applications/cups.desktop
	rm -f /usr/share/applications/uxterm.desktop
	rm -f /usr/share/applications/sbackup-restore-su.desktop
	rm -f /usr/share/applications/sbackup-config-su.desktop
	rm -f /usr/share/applications/designer-qt4.desktop
	rm -f /usr/share/applications/linguist-qt4.desktop
	rm -f /usr/share/applications/assistant-qt4.desktop
	rm -f /usr/share/applications/qdbusviewer-qt4.desktop
	rm -f /usr/share/applications/qtconfig-qt4.desktop
	rm -f /usr/share/applications/nvidia-settings.desktop
	rm -f /usr/share/applications/hplip.desktop
#Switch Icons
	sed -i 's@Icon=xterm-color_48x48@Icon=xorg@' /usr/share/applications/xterm.desktop
	sed -i 's@Icon=tracker@Icon=preferences-system-search@' /usr/share/applications/tracker-preferences.desktop
	sed -i 's@Icon=sbackup-restore@Icon=grsync-restore@' /usr/share/applications/sbackup-restore.desktop
	sed -i 's@Icon=sbackup-conf@Icon=grsync@' /usr/share/applications/sbackup-config.desktop
#Switch Icons from Apricity Assets
	cp -f /etc/apricity-assets/playonlinux.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux15.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux16.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux22.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux32.png /usr/share/playonlinux/etc
#Remove Arch Installation Instructions
	rm -f /root/install.txt
#Set Initcpio
	echo "$(cat /etc/mkinitcpio.conf)"
#Enable Services
	systemctl enable org.cups.cupsd.service
	systemctl enable smbd nmbd
	#systemctl enable bumblebeed
	systemctl enable graphical.target gdm.service pacman-init.service dhcpcd.service
	echo 'Enabled dhcpd, gdm'
	systemctl enable bluetooth.service
	echo 'Enabled bluetooth'
	systemctl enable avahi-daemon.service
	echo 'Enabled avahi'
	systemctl -fq enable NetworkManager ModemManager
	echo 'Enabled network'
	systemctl mask systemd-rfkill@.service
	systemctl set-default graphical.target
#Edit Mirrorlist
	sed -i "s/#Server/Server/g" /etc/pacman.d/mirrorlist
	sed -i 's/#\(Storage=\)auto/\1volatile/' /etc/systemd/journald.conf
#Enable Calamares Autostart
	mkdir -p /home/liveuser/.config/autostart
	cp -f /usr/share/applications/calamares.desktop /home/liveuser/.config/autostart/calamares.desktop
#Enable ICE+Chrome Stable
	ln -sf /usr/bin/google-chrome-stable /usr/bin/google-chrome
	export _BROWSER=google-chrome-stable
	echo "BROWSER=/usr/bin/${_BROWSER}" >> /etc/environment
	echo "BROWSER=/usr/bin/${_BROWSER}" >> /etc/skel/.bashrc
	echo "BROWSER=/usr/bin/${_BROWSER}" >> /etc/profile
#Set Nano Editor
	export _EDITOR=nano
	echo "EDITOR=${_EDITOR}" >> /etc/environment
	echo "EDITOR=${_EDITOR}" >> /etc/skel/.bashrc
	echo "EDITOR=${_EDITOR}" >> /etc/profile
#Enable Sudo
	chmod 750 /etc/sudoers.d
	chmod 440 /etc/sudoers.d/g_wheel
	chown -R root /etc/sudoers.d
	echo "Enabled Sudo"
#Set Apricity Grub Theme
	/etc/apricity-assets/Elegant_Dark/install.sh
#Enable Apricity Plymouth Theme
	sed -i.bak 's/base udev/base udev plymouth/g' /etc/mkinitcpio.conf
	chown -R root.root /usr/share/plymouth/themes/apricity
	plymouth-set-default-theme -R apricity
	chsh -s /bin/zsh
#Setup Pacman
	pacman-key --init archlinux
	pacman-key --populate archlinux
	pacman-key --refresh-keys
#Setup Vim
	cp -r /etc/skel/.vim /root/.vim
	cp /etc/skel/.vimrc /root/.vimrc
	cp -r /etc/skel/.vim /home/liveuser/.vim
	cp /etc/skel/.vimrc /home/liveuser/.vimrc
#Set Cinnamon
    gsettings set org.gnome.desktop.session session-name cinnamon
    cp -f /etc/apricity-tmp/custom.conf /etc/gdm
    rm -f /usr/share/xsessions/gnome.desktop
    rm -f /usr/share/xsessions/gnome-classic.desktop
    echo -e "#!/usr/bin/env xdg-open\n$(cat /home/liveuser/.config/autostart/calamares.desktop)" > /home/liveuser/.config/autostart/calamares.desktop
    chmod +x /home/liveuser/.config/autostart/calamares.desktop
else
	echo "i686"
fi

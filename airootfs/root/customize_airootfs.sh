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
chown -R root /etc/sudoers.d
chmod -R 755 /etc/sudoers.d

#Create Liveuser
id -u liveuser &>/dev/null || useradd -m "liveuser" -g users -G "adm,audio,floppy,log,network,rfkill,scanner,storage,optical,power,wheel"
passwd -d liveuser
#rm /home/liveuser/.config/autostart/firstrun.desktop
echo 'Created User'

# Setup Pacman
# pacman-key --init archlinux
# pacman-key --populate archlinux
# pacman-key --init
# pacman-key --init apricity
# pacman-key --populate apricity
# pacman-key --populate
# pacman -Syy
# pacman-key --refresh-keys

#Edit Mirrorlist
	sed -i "s/#Server/Server/g" /etc/pacman.d/mirrorlist
	sed -i 's/#\(Storage=\)auto/\1volatile/' /etc/systemd/journald.conf

#Load freezedry configuration
sudo -u liveuser freezedry --load /etc/freezedry/default.toml --livecd || /bin/true

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
sed -i 's@Icon=/usr/share/hplip/data/images/128x128/hp_logo.png@Icon=hplip@' /usr/share/applications/hplip.desktop || /bin/true
	cp -f /etc/apricity-assets/playonlinux.png /usr/share/playonlinux/etc || /bin/true
	cp -f /etc/apricity-assets/playonlinux15.png /usr/share/playonlinux/etc || /bin/true
	cp -f /etc/apricity-assets/playonlinux16.png /usr/share/playonlinux/etc || /bin/true
	cp -f /etc/apricity-assets/playonlinux22.png /usr/share/playonlinux/etc || /bin/true
	cp -f /etc/apricity-assets/playonlinux32.png /usr/share/playonlinux/etc || /bin/true
	rm -f /root/install.txt
	echo "$(cat /etc/mkinitcpio.conf)"
#Enable Calamares Autostart
	mkdir -p /home/liveuser/.config/autostart
	ln -fs /usr/share/applications/calamares.desktop /home/liveuser/.config/autostart/calamares.desktop
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
	/etc/apricity-assets/Elegant_Dark/install.sh || /bin/true
#Enable Apricity Plymouth Theme
	#sed -i.bak 's/base udev/base udev plymouth/g' /etc/mkinitcpio.conf
	#chown -R root.root /usr/share/plymouth/themes/apricity
	#plymouth-set-default-theme -R apricity
	chsh -s /bin/zsh || /bin/true
#Setup Su
    sed -i /etc/pam.d/su -e 's/auth      sufficient  pam_wheel.so trust use_uid/#auth        sufficient  pam_wheel.so trust use_uid/'
#Try to do sudo again
    chmod -R 755 /etc/sudoers.d
else
	echo "i686"
sed -i 's@Icon=/usr/share/hplip/data/images/128x128/hp_logo.png@Icon=hplip@' /usr/share/applications/hplip.desktop || /bin/true
	cp -f /etc/apricity-assets/playonlinux.png /usr/share/playonlinux/etc || /bin/true
	cp -f /etc/apricity-assets/playonlinux15.png /usr/share/playonlinux/etc || /bin/true
	cp -f /etc/apricity-assets/playonlinux16.png /usr/share/playonlinux/etc || /bin/true
	cp -f /etc/apricity-assets/playonlinux22.png /usr/share/playonlinux/etc || /bin/true
	cp -f /etc/apricity-assets/playonlinux32.png /usr/share/playonlinux/etc || /bin/true
	rm -f /root/install.txt
	echo "$(cat /etc/mkinitcpio.conf)"
#Enable Calamares Autostart
	mkdir -p /home/liveuser/.config/autostart
	ln -fs /usr/share/applications/calamares.desktop /home/liveuser/.config/autostart/calamares.desktop
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
	/etc/apricity-assets/Elegant_Dark/install.sh || /bin/true
#Enable Apricity Plymouth Theme
	#sed -i.bak 's/base udev/base udev plymouth/g' /etc/mkinitcpio.conf
	#chown -R root.root /usr/share/plymouth/themes/apricity
	#plymouth-set-default-theme -R apricity
	chsh -s /bin/zsh || /bin/true
#Setup Su
    sed -i /etc/pam.d/su -e 's/auth      sufficient  pam_wheel.so trust use_uid/#auth        sufficient  pam_wheel.so trust use_uid/'
#Try to do sudo again
    chmod -R 755 /etc/sudoers.d
fi

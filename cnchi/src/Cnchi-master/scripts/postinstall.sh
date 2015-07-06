#!/usr/bin/bash

# -*- coding: utf-8 -*-
#
#  postinstall.sh
#
#  Copyright © 2013,2014 Apricity OS
#
#  This file is part of Cnchi.
#
#  Cnchi is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  Cnchi is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Cnchi; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.

set_xorg()
{
    cp /usr/share/cnchi/scripts/postinstall/50-synaptics.conf ${DESTDIR}/etc/X11/xorg.conf.d/50-synaptics.conf
    cp /usr/share/cnchi/scripts/postinstall/99-killX.conf ${DESTDIR}/etc/X11/xorg.conf.d/99-killX.conf
}

gnome_settings()
{
##############################################################################################
	rm /usr/share/applications/bssh.desktop
	rm /usr/share/applications/bvnc.desktop
	rm /usr/share/applications/avahi-discover.desktop
	rm /usr/share/applications/qv4l2.desktop
	rm /usr/share/applications/polkit-gnome-authentication-agent-1.desktop
	rm /usr/share/applications/tracker-needle.desktop
	rm /usr/share/applications/zenmap.desktop
	rm /usr/share/applications/zenmap-root.desktop
	rm /usr/share/applications/gksu.desktop
	rm /usr/share/applications/gucharmap.desktop
	rm /usr/share/applications/cups.desktop
	rm /usr/share/applications/uxterm.desktop
	rm /usr/share/applications/epiphany.desktop
	rm /usr/share/applications/empathy.desktop
	rm /usr/share/applications/designer-qt4.desktop
	rm /usr/share/applications/linguist-qt4.desktop
	rm /usr/share/applications/assistant-qt4.desktop
	rm /usr/share/applications/qdbusviewer-qt4.desktop
	sed -i 's@/usr/share/argon/argon.png@gnome-app-install@' /usr/share/applications/argon.desktop
	sed -i 's@/usr/share/argon/argon.png@gnome-app-install@' /usr/share/applications/argon-notifier-config.desktop
	sed -i 's@Icon=gnome-books@Icon=unity-webapps-amazoncloudreader@' /usr/share/applications/org.gnome.Books.desktop
	sed -i 's@Icon=builder@Icon=textwrangler@' /usr/share/applications/org.gnome.Builder.desktop
	sed -i 's@Icon=gnome-characters@Icon=accessories-character-map@' /usr/share/applications/org.gnome.Characters.desktop
	sed -i 's@Icon=x-office-address-book@Icon=evolution-addressbook@' /usr/share/applications/org.gnome.Contacts.desktop
	sed -i 's@Icon=grsync.png@Icon=luckybackup@' /usr/share/applications/grsync.desktop
	sed -i 's@Icon=xterm-color_48x48@Icon=xorg@' /usr/share/applications/xterm.desktop
	sed -i 's@Icon=tracker@Icon=preferences-system-search@' /usr/share/applications/tracker-preferences.desktop
	cp -f /etc/apricity-assets/playonlinux.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux15.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux16.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux22.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux32.png /usr/share/playonlinux/etc
	set -e -u
	sed -i 's/#\(en_US\.UTF-8\)/\1/' /etc/locale.gen
	locale-gen
	ln -sf /usr/share/zoneinfo/UTC /etc/localtime
	usermod -s /usr/bin/zsh root
	cp -aT /etc/skel/ /root/
	chmod 700 /root
	id -u liveuser &>/dev/null || useradd -m "liveuser" -g users -G "adm,audio,floppy,log,network,rfkill,scanner,storage,optical,power,wheel"
	passwd -d liveuser
	echo 'created user'
	chmod 750 /etc/sudoers.d
	chmod 440 /etc/sudoers.d/g_wheel
	chown -R root /etc/sudoers.d
	echo 'sudoed'
	EDITOR=nano
	/etc/apricity-assets/Elegant_Dark/install.sh
	sed -i.bak 's/Arch Linux/Apricity OS/g' /usr/lib/os-release
	sed -i.bak 's/arch/apricity/g' /usr/lib/os-release
	sed -i.bak 's/www.archlinux.org/www.apricityos.com/g' /usr/lib/os-release
	sed -i.bak 's/bbs.archlinux.org/www.apricityos.com/g' /usr/lib/os-release
	sed -i.bak 's/bugs.archlinux.org/www.apricityos.com/g' /usr/lib/os-release
	cp /usr/lib/os-release /etc/os-release
	arch=`uname -m`
	if [ "$arch" == "x86_64" ]
	then
	    echo "x86_64, plymouth okay"
	    echo "$(cat /etc/mkinitcpio.conf)"
	    sed -i.bak 's/base udev/base udev plymouth/g' /etc/mkinitcpio.conf
	    chown -R root.root /usr/share/plymouth/themes/apricity
	    plymouth-set-default-theme -R apricity
	    mkinitcpio -p linux
	    systemctl enable org.cups.cupsd.service
	    systemctl enable smbd nmbd
	else
	    echo "i686, no plymouth"
	fi
	sed -i "s/#Server/Server/g" /etc/pacman.d/mirrorlist
	sed -i 's/#\(Storage=\)auto/\1volatile/' /etc/systemd/journald.conf
	echo 'seded'
	systemctl enable graphical.target gdm.service pacman-init.service dhcpcd.service
	systemctl -fq enable NetworkManager ModemManager
	systemctl set-default graphical.target
	systemctl enable tlp.service tlp-sleep.service
	systemctl mask systemd-rfkill@.service
	cp -R ${DESTDIR}/home/${USER_NAME}/.config ${DESTDIR}/etc/skel
	chroot ${DESTDIR} su -c xdg-user-dirs-update ${USER_NAME}
	if [[ "${KEYBOARD_VARIANT}" != '' ]]; then
		sed -i "s/'us'/'${KEYBOARD_LAYOUT}+${KEYBOARD_VARIANT}'/" /usr/share/cnchi/scripts/set-settings
	else
		sed -i "s/'us'/'${KEYBOARD_LAYOUT}'/" /usr/share/cnchi/scripts/set-settings
	fi
}

cinnamon_settings()
{
    
}

xfce_settings()
{
   

}

openbox_settings()
{
    
}

lxde_settings()
{
   
}

lxqt_settings()
{

}

kde4_settings()
{
    
}

plasma5_settings()
{
    
}

mate_settings()
{
    
}

nox_settings()
{
##############################################################################################
	rm /usr/share/applications/bssh.desktop
	rm /usr/share/applications/bvnc.desktop
	rm /usr/share/applications/avahi-discover.desktop
	rm /usr/share/applications/qv4l2.desktop
	rm /usr/share/applications/polkit-gnome-authentication-agent-1.desktop
	rm /usr/share/applications/tracker-needle.desktop
	rm /usr/share/applications/zenmap.desktop
	rm /usr/share/applications/zenmap-root.desktop
	rm /usr/share/applications/gksu.desktop
	rm /usr/share/applications/gucharmap.desktop
	rm /usr/share/applications/cups.desktop
	rm /usr/share/applications/uxterm.desktop
	rm /usr/share/applications/epiphany.desktop
	rm /usr/share/applications/empathy.desktop
	rm /usr/share/applications/designer-qt4.desktop
	rm /usr/share/applications/linguist-qt4.desktop
	rm /usr/share/applications/assistant-qt4.desktop
	rm /usr/share/applications/qdbusviewer-qt4.desktop
	sed -i 's@/usr/share/argon/argon.png@gnome-app-install@' /usr/share/applications/argon.desktop
	sed -i 's@/usr/share/argon/argon.png@gnome-app-install@' /usr/share/applications/argon-notifier-config.desktop
	sed -i 's@Icon=gnome-books@Icon=unity-webapps-amazoncloudreader@' /usr/share/applications/org.gnome.Books.desktop
	sed -i 's@Icon=builder@Icon=textwrangler@' /usr/share/applications/org.gnome.Builder.desktop
	sed -i 's@Icon=gnome-characters@Icon=accessories-character-map@' /usr/share/applications/org.gnome.Characters.desktop
	sed -i 's@Icon=x-office-address-book@Icon=evolution-addressbook@' /usr/share/applications/org.gnome.Contacts.desktop
	sed -i 's@Icon=grsync.png@Icon=luckybackup@' /usr/share/applications/grsync.desktop
	sed -i 's@Icon=xterm-color_48x48@Icon=xorg@' /usr/share/applications/xterm.desktop
	sed -i 's@Icon=tracker@Icon=preferences-system-search@' /usr/share/applications/tracker-preferences.desktop
	cp -f /etc/apricity-assets/playonlinux.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux15.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux16.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux22.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux32.png /usr/share/playonlinux/etc
	set -e -u
	sed -i 's/#\(en_US\.UTF-8\)/\1/' /etc/locale.gen
	locale-gen
	ln -sf /usr/share/zoneinfo/UTC /etc/localtime
	usermod -s /usr/bin/zsh root
	cp -aT /etc/skel/ /root/
	chmod 700 /root
	id -u liveuser &>/dev/null || useradd -m "liveuser" -g users -G "adm,audio,floppy,log,network,rfkill,scanner,storage,optical,power,wheel"
	passwd -d liveuser
	echo 'created user'
	chmod 750 /etc/sudoers.d
	chmod 440 /etc/sudoers.d/g_wheel
	chown -R root /etc/sudoers.d
	echo 'sudoed'
	EDITOR=nano
	/etc/apricity-assets/Elegant_Dark/install.sh
	sed -i.bak 's/Arch Linux/Apricity OS/g' /usr/lib/os-release
	sed -i.bak 's/arch/apricity/g' /usr/lib/os-release
	sed -i.bak 's/www.archlinux.org/www.apricityos.com/g' /usr/lib/os-release
	sed -i.bak 's/bbs.archlinux.org/www.apricityos.com/g' /usr/lib/os-release
	sed -i.bak 's/bugs.archlinux.org/www.apricityos.com/g' /usr/lib/os-release
	cp /usr/lib/os-release /etc/os-release
	arch=`uname -m`
	if [ "$arch" == "x86_64" ]
	then
	    echo "x86_64, plymouth okay"
	    echo "$(cat /etc/mkinitcpio.conf)"
	    sed -i.bak 's/base udev/base udev plymouth/g' /etc/mkinitcpio.conf
	    chown -R root.root /usr/share/plymouth/themes/apricity
	    plymouth-set-default-theme -R apricity
	    mkinitcpio -p linux
	    systemctl enable org.cups.cupsd.service
	    systemctl enable smbd nmbd
	else
	    echo "i686, no plymouth"
	fi
	sed -i "s/#Server/Server/g" /etc/pacman.d/mirrorlist
	sed -i 's/#\(Storage=\)auto/\1volatile/' /etc/systemd/journald.conf
	echo 'seded'
	systemctl enable graphical.target gdm.service pacman-init.service dhcpcd.service
	systemctl -fq enable NetworkManager ModemManager
	systemctl set-default graphical.target
	systemctl enable tlp.service tlp-sleep.service
	systemctl mask systemd-rfkill@.service
	cp -R ${DESTDIR}/home/${USER_NAME}/.config ${DESTDIR}/etc/skel
	chroot ${DESTDIR} su -c xdg-user-dirs-update ${USER_NAME}
	if [[ "${KEYBOARD_VARIANT}" != '' ]]; then
		sed -i "s/'us'/'${KEYBOARD_LAYOUT}+${KEYBOARD_VARIANT}'/" /usr/share/cnchi/scripts/set-settings
	else
		sed -i "s/'us'/'${KEYBOARD_LAYOUT}'/" /usr/share/cnchi/scripts/set-settings
	fi
}

enlightenment_settings()
{

}

postinstall()
{
##############################################################################################
    USER_NAME=$1
    DESTDIR=$2
    DESKTOP=$3
    KEYBOARD_LAYOUT=$4
    KEYBOARD_VARIANT=$5
    IS_VBOX=$6
    # Specific user configurations
    if [[ -f /usr/share/applications/firefox.desktop ]]; then
        export _BROWSER=firefox
    else
        export _BROWSER=chromium
    fi

    ## Set desktop-specific settings
    "${DESKTOP}_settings"

    ## Workaround for LightDM bug https://bugs.launchpad.net/lightdm/+bug/1069218
    chroot ${DESTDIR} sed -i 's|UserAccounts|UserList|g' /etc/lightdm/users.conf

    ## Unmute alsa channels
    chroot ${DESTDIR} amixer -c 0 set Master playback 50% unmute > /dev/null 2>&1

    # Fix transmission leftover
    if [ -f ${DESTDIR}/usr/lib/tmpfiles.d/transmission.conf ]; then
        mv ${DESTDIR}/usr/lib/tmpfiles.d/transmission.conf ${DESTDIR}/usr/lib/tmpfiles.d/transmission.conf.backup
    fi

    # Configure touchpad. Skip with base installs
    if [[ $DESKTOP != 'nox' ]]; then
        set_xorg
    fi

    # Configure fontconfig
    FONTCONFIG_FILE="/usr/share/cnchi/scripts/fonts.conf"
    FONTCONFIG_DIR="${DESTDIR}/home/${USER_NAME}/.config/fontconfig"
    if [ -f ${FONTCONFIG_FILE} ]; then
        mkdir -p ${FONTCONFIG_DIR}
        cp ${FONTCONFIG_FILE} ${FONTCONFIG_DIR}
    fi

    # Set Apricity name in filesystem files
    cp /etc/arch-release ${DESTDIR}/etc
    cp /etc/os-release ${DESTDIR}/etc
    #cp /etc/lsb-release ${DESTDIR}/etc
    sed -i 's|Arch|Apricity|g' ${DESTDIR}/etc/issue

    # Set BROWSER var
    echo "BROWSER=/usr/bin/${_BROWSER}" >> ${DESTDIR}/etc/environment
    echo "BROWSER=/usr/bin/${_BROWSER}" >> ${DESTDIR}/etc/skel/.bashrc
    echo "BROWSER=/usr/bin/${_BROWSER}" >> ${DESTDIR}/etc/profile

    # Configure makepkg so that it doesn't compress packages after building.
    # Most users are building packages to install them locally so there's no need for compression.
    sed -i "s|^PKGEXT='.pkg.tar.xz'|PKGEXT='.pkg.tar'|g" /etc/makepkg.conf

    # Set lightdm-webkit2-greeter in lightdm.conf. This should have been done here (not in the pkg) all along.
    sed -i 's|#greeter-session=example-gtk-gnome|greeter-session=lightdm-webkit2-greeter|g' ${DESTDIR}/etc/lightdm/lightdm.conf

    ## Ensure user permissions are set in /home
    chroot ${DESTDIR} chown -R ${USER_NAME}:users /home/${USER_NAME}

    # Start vbox client services if we are installed in vbox
    if [[ $IS_VBOX ]] || [[ $IS_VBOX = 0 ]] || [[ $IS_VBOX = "True" ]]; then
        sed -i 's|echo "X|/usr/bin/VBoxClient-all \&\necho "X|g' ${DESTDIR}/etc/lightdm/Xsession
    fi

	rm /usr/share/applications/bssh.desktop
	rm /usr/share/applications/bvnc.desktop
	rm /usr/share/applications/avahi-discover.desktop
	rm /usr/share/applications/qv4l2.desktop
	rm /usr/share/applications/polkit-gnome-authentication-agent-1.desktop
	rm /usr/share/applications/tracker-needle.desktop
	rm /usr/share/applications/zenmap.desktop
	rm /usr/share/applications/zenmap-root.desktop
	rm /usr/share/applications/gksu.desktop
	rm /usr/share/applications/gucharmap.desktop
	rm /usr/share/applications/cups.desktop
	rm /usr/share/applications/uxterm.desktop
	rm /usr/share/applications/epiphany.desktop
	rm /usr/share/applications/empathy.desktop
	rm /usr/share/applications/designer-qt4.desktop
	rm /usr/share/applications/linguist-qt4.desktop
	rm /usr/share/applications/assistant-qt4.desktop
	rm /usr/share/applications/qdbusviewer-qt4.desktop
	sed -i 's@/usr/share/argon/argon.png@gnome-app-install@' /usr/share/applications/argon.desktop
	sed -i 's@/usr/share/argon/argon.png@gnome-app-install@' /usr/share/applications/argon-notifier-config.desktop
	sed -i 's@Icon=gnome-books@Icon=unity-webapps-amazoncloudreader@' /usr/share/applications/org.gnome.Books.desktop
	sed -i 's@Icon=builder@Icon=textwrangler@' /usr/share/applications/org.gnome.Builder.desktop
	sed -i 's@Icon=gnome-characters@Icon=accessories-character-map@' /usr/share/applications/org.gnome.Characters.desktop
	sed -i 's@Icon=x-office-address-book@Icon=evolution-addressbook@' /usr/share/applications/org.gnome.Contacts.desktop
	sed -i 's@Icon=grsync.png@Icon=luckybackup@' /usr/share/applications/grsync.desktop
	sed -i 's@Icon=xterm-color_48x48@Icon=xorg@' /usr/share/applications/xterm.desktop
	sed -i 's@Icon=tracker@Icon=preferences-system-search@' /usr/share/applications/tracker-preferences.desktop
	cp -f /etc/apricity-assets/playonlinux.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux15.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux16.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux22.png /usr/share/playonlinux/etc
	cp -f /etc/apricity-assets/playonlinux32.png /usr/share/playonlinux/etc
	set -e -u
	sed -i 's/#\(en_US\.UTF-8\)/\1/' /etc/locale.gen
	locale-gen
	ln -sf /usr/share/zoneinfo/UTC /etc/localtime
	usermod -s /usr/bin/zsh root
	cp -aT /etc/skel/ /root/
	chmod 700 /root
	id -u liveuser &>/dev/null || useradd -m "liveuser" -g users -G "adm,audio,floppy,log,network,rfkill,scanner,storage,optical,power,wheel"
	passwd -d liveuser
	echo 'created user'
	chmod 750 /etc/sudoers.d
	chmod 440 /etc/sudoers.d/g_wheel
	chown -R root /etc/sudoers.d
	echo 'sudoed'
	EDITOR=nano
	/etc/apricity-assets/Elegant_Dark/install.sh
	sed -i.bak 's/Arch Linux/Apricity OS/g' /usr/lib/os-release
	sed -i.bak 's/arch/apricity/g' /usr/lib/os-release
	sed -i.bak 's/www.archlinux.org/www.apricityos.com/g' /usr/lib/os-release
	sed -i.bak 's/bbs.archlinux.org/www.apricityos.com/g' /usr/lib/os-release
	sed -i.bak 's/bugs.archlinux.org/www.apricityos.com/g' /usr/lib/os-release
	cp /usr/lib/os-release /etc/os-release
	arch=`uname -m`
	if [ "$arch" == "x86_64" ]
	then
	    echo "x86_64, plymouth okay"
	    echo "$(cat /etc/mkinitcpio.conf)"
	    sed -i.bak 's/base udev/base udev plymouth/g' /etc/mkinitcpio.conf
	    chown -R root.root /usr/share/plymouth/themes/apricity
	    plymouth-set-default-theme -R apricity
	    mkinitcpio -p linux
	    systemctl enable org.cups.cupsd.service
	    systemctl enable smbd nmbd
	else
	    echo "i686, no plymouth"
	fi
	sed -i "s/#Server/Server/g" /etc/pacman.d/mirrorlist
	sed -i 's/#\(Storage=\)auto/\1volatile/' /etc/systemd/journald.conf
	echo 'seded'
	systemctl enable graphical.target gdm.service pacman-init.service dhcpcd.service
	systemctl -fq enable NetworkManager ModemManager
	systemctl set-default graphical.target
	systemctl enable tlp.service tlp-sleep.service
	systemctl mask systemd-rfkill@.service
	cp -R ${DESTDIR}/home/${USER_NAME}/.config ${DESTDIR}/etc/skel
	chroot ${DESTDIR} su -c xdg-user-dirs-update ${USER_NAME}
	if [[ "${KEYBOARD_VARIANT}" != '' ]]; then
		sed -i "s/'us'/'${KEYBOARD_LAYOUT}+${KEYBOARD_VARIANT}'/" /usr/share/cnchi/scripts/set-settings
	else
		sed -i "s/'us'/'${KEYBOARD_LAYOUT}'/" /usr/share/cnchi/scripts/set-settings
	fi
}

touch /tmp/.postinstall.lock
postinstall $1 $2 $3 $4 $5 $6 > /tmp/postinstall.log 2>&1
rm /tmp/.postinstall.lock

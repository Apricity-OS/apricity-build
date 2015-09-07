#!/usr/bin/bash

# -*- coding: utf-8 -*-
#
#  postinstall.sh
#
#  Copyright © 2015 Apricity
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
    cd
}

cinnamon_settings()
{
    cd
}

xfce_settings()
{
    cd

}

openbox_settings()
{
    cd
}

lxde_settings()
{
    cd
}

lxqt_settings()
{
    cd
}

kde4_settings()
{
    cd
}

plasma5_settings()
{
    cd
}

mate_settings()
{
    cd
}

nox_settings()
{
    cd
}

enlightenment_settings()
{
    cd
}

postinstall()
{
    USER_NAME=$1
    DESTDIR=$2
    DESKTOP=$3
    KEYBOARD_LAYOUT=$4
    KEYBOARD_VARIANT=$5
    IS_VBOX=$6

    export _BROWSER=google-chrome-beta

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

    # Set BROWSER var
    echo "BROWSER=/usr/bin/${_BROWSER}" >> ${DESTDIR}/etc/environment
    echo "BROWSER=/usr/bin/${_BROWSER}" >> ${DESTDIR}/etc/skel/.bashrc
    echo "BROWSER=/usr/bin/${_BROWSER}" >> ${DESTDIR}/etc/profile

    # Configure makepkg so that it doesn't compress packages after building.
    # Most users are building packages to install them locally so there's no need for compression.
    sed -i "s|^PKGEXT='.pkg.tar.xz'|PKGEXT='.pkg.tar'|g" /etc/makepkg.conf

    ## Ensure user permissions are set in /home
    chroot ${DESTDIR} chown -R ${USER_NAME}:users /home/${USER_NAME}

    cp -f /etc/default/keyboard ${DESTDIR}/etc/default/keyboard
    # Set gsettings input-source
    if [[ "${KEYBOARD_VARIANT}" != '' ]]; then
        sed -i "s/'us'/'${KEYBOARD_LAYOUT}+${KEYBOARD_VARIANT}'/" ${DESTDIR}/etc/skel/.config/autostart/firstrun.sh
        sed -i "s/'us'/'${KEYBOARD_LAYOUT}+${KEYBOARD_VARIANT}'/" ${DESTDIR}/home/${USER_NAME}/.config/autostart/firstrun.sh
        sed -i "s/us/'${KEYBOARD_LAYOUT}+${KEYBOARD_VARIANT}'/" ${DESTDIR}/etc/apricity-assets/10-evdev.conf
        sed -i "s/variant/${KEYBOARD_VARIANT}/" ${DESTDIR}/etc/default/keyboard
        sed -i "s/us/${KEYBOARD_LAYOUT}/" ${DESTDIR}/etc/default/keyboard
	echo "setxkbmap ${KEYBOARD_LAYOUT} -variant ${KEYBOARD_VARIANT}" >> ${DESTDIR}/etc/gdm/Init/Default

    else
        sed -i "s/'us'/'${KEYBOARD_LAYOUT}'/" ${DESTDIR}/etc/skel/.config/autostart/firstrun.sh
        sed -i "s/'us'/'${KEYBOARD_LAYOUT}'/" ${DESTDIR}/home/${USER_NAME}/.config/autostart/firstrun.sh
        sed -i "s/us/'${KEYBOARD_LAYOUT}'/" ${DESTDIR}/etc/apricity-assets/10-evdev.conf
        sed -i "s/us/'${KEYBOARD_LAYOUT}'/" ${DESTDIR}/etc/default/keyboard
        sed -i "s/variant//" ${DESTDIR}/etc/default/keyboard
	echo "setxkbmap ${KEYBOARD_LAYOUT}" >> ${DESTDIR}/etc/gdm/Init/Default
    fi

    # Set gsettings
    rm ${DESTDIR}/usr/share/applications/bssh.desktop
	rm ${DESTDIR}/usr/share/applications/bvnc.desktop
	rm ${DESTDIR}/usr/share/applications/avahi-discover.desktop
	rm ${DESTDIR}/usr/share/applications/qv4l2.desktop
	rm ${DESTDIR}/usr/share/applications/polkit-gnome-authentication-agent-1.desktop
	rm ${DESTDIR}/usr/share/applications/tracker-needle.desktop
	rm ${DESTDIR}/usr/share/applications/gksu.desktop
	rm ${DESTDIR}/usr/share/applications/gucharmap.desktop
	rm ${DESTDIR}/usr/share/applications/cups.desktop
	rm ${DESTDIR}/usr/share/applications/uxterm.desktop
	rm ${DESTDIR}/usr/share/applications/sbackup-restore-su.desktop
	rm ${DESTDIR}/usr/share/applications/sbackup-config-su.desktop

	rm ${DESTDIR}/usr/share/gnome-background-properties/adwaita.xml
	rm ${DESTDIR}/usr/share/gnome-background-properties/gnome-default.xml

	cp -f ${DESTDIR}/usr/share/gnome-background-properties/apricity-backgrounds.xml ${DESTDIR}/usr/share/gnome-background-properties/gnome-backgrounds.xml

	chmod 755 -R ${DESTDIR}/etc/apricity-assets/google-chrome-beta
	ln -s ${DESTDIR}/usr/bin/google-chrome-beta ${DESTDIR}/usr/bin/google-chrome

	sed -i 's@/usr/share/argon/argon.png@gnome-app-install@' ${DESTDIR}/usr/share/applications/argon.desktop
	sed -i 's@/usr/share/argon/argon.png@gnome-app-install@' ${DESTDIR}/usr/share/applications/argon-notifier-config.desktop
	sed -i 's@Icon=x-office-address-book@Icon=evolution-addressbook@' ${DESTDIR}/usr/share/applications/org.gnome.Contacts.desktop
	sed -i 's@Icon=xterm-color_48x48@Icon=xorg@' ${DESTDIR}/usr/share/applications/xterm.desktop
	sed -i 's@Icon=tracker@Icon=preferences-system-search@' ${DESTDIR}/usr/share/applications/tracker-preferences.desktop
	sed -i 's@Icon=sbackup-restore@Icon=grsync-restore@' ${DESTDIR}/usr/share/applications/sbackup-restore.desktop
	sed -i 's@Icon=sbackup-conf@Icon=grsync@' ${DESTDIR}/usr/share/applications/sbackup-config.desktop
	cp -f ${DESTDIR}/etc/apricity-assets/playonlinux.png ${DESTDIR}/usr/share/playonlinux/etc
	cp -f ${DESTDIR}/etc/apricity-assets/playonlinux15.png ${DESTDIR}/usr/share/playonlinux/etc
	cp -f ${DESTDIR}/etc/apricity-assets/playonlinux16.png ${DESTDIR}/usr/share/playonlinux/etc
	cp -f ${DESTDIR}/etc/apricity-assets/playonlinux22.png ${DESTDIR}/usr/share/playonlinux/etc
	cp -f ${DESTDIR}/etc/apricity-assets/playonlinux32.png ${DESTDIR}/usr/share/playonlinux/etc
	sed -i.bak 's/Arch Linux/Apricity OS/g' ${DESTDIR}/usr/lib/os-release
	sed -i.bak 's/arch/apricity/g' ${DESTDIR}/usr/lib/os-release
	sed -i.bak 's/www.archlinux.org/www.apricityos.com/g' ${DESTDIR}/usr/lib/os-release
	sed -i.bak 's/bbs.archlinux.org/www.apricityos.com/g' ${DESTDIR}/usr/lib/os-release
	sed -i.bak 's/bugs.archlinux.org/www.apricityos.com/g' ${DESTDIR}/usr/lib/os-release
	cp -f ${DESTDIR}/usr/lib/os-release ${DESTDIR}/etc/os-release
	sed -i.bak 's/base udev/base udev plymouth/g' ${DESTDIR}/etc/mkinitcpio.conf
	chown -R root.root ${DESTDIR}/usr/share/plymouth/themes/apricity
	cp -R ${DESTDIR}/home/${USER_NAME}/.config ${DESTDIR}/etc/skel
	chroot ${DESTDIR} su -c xdg-user-dirs-update ${USER_NAME}
	cp -rf /usr/share/gnome-shell/extensions/* ${DESTDIR}/usr/share/gnome-shell/extensions

}

touch /tmp/.postinstall.lock
postinstall $1 $2 $3 $4 $5 $6 > /tmp/postinstall.log 2>&1
rm /tmp/.postinstall.lock

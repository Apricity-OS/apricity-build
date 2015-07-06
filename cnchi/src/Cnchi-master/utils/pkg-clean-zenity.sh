#!/bin/bash

#remove="pacman -Rc cinnamon --noconfirm"
#remove2="pacman -Rcns gnome --noconfirm"
#remove3="pacman -Rcn gnome --noconfirm"
#remove4="pacman -Rc gnome--nosave --noconfirm"
#clean="pacman -Scc --noconfirm"
#clean2="pacman -Syy --noconfirm"

#install_cinnamon="pacman -S antergos-gnome-defaults-list cdrkit cinnamon cinnamon-session cinnamon-control-center cinnamon-desktop cinnamon-settings-daemon cinnamon-menus cinnamon-screensaver cinnamon-theme-nadia cjs empathy eog evince faenza-icon-theme file-roller gedit gnome-calculator gnome-disk-utility gnome-keyring gnome-screenshot gnome-system-monitor gnome-terminal gnome-themes-standard gst-libav gst-plugins-bad gst-plugins-base gst-plugins-good gst-plugins-ugly gst-vaapi gstreamer0.10-plugins gstreamer0.10-vaapi gvfs-mtp gvfs-smb hicolor-icon-theme libgnomeui muffin nemo network-manager-applet networkmanager-openvpn networkmanager-pptp telepathy totem transmission-gtk xdg-user-dirs-gtk xfburn xnoise zukitwo-themes lightdm lightdm-webkit-greeter lightdm-webkit-theme-antergos --noconfirm"
#install_kde="pacman -S kde-meta-kdebase kde-meta-kdenetwork kdeplasma-applets-plasma-nm apper cdrdao digikam dvd+rw-tools emovix grub2-editor k3b kde-gtk-config kde-meta-kdeartwork kde-telepathy-meta kdeadmin-kuser kdegraphics-gwenview kdegraphics-ksnapshot kdegraphics-okular kdegraphics-thumbnailers kdemultimedia-ffmpegthumbs kdemultimedia-kmix kdepim-kmail kdepim-kontact kdeplasma-addons-applets-lancelot kdeplasma-addons-applets-notes kdesdk-dolphin-plugins kdesdk-kate kdeutils-ark kdeutils-kgpg kdeutils-kwalletmanager kdeutils-sweeper kipi-plugins kwebkitpart kfaenza-icon-theme oxygen-gtk2 oxygen-gtk3 qt5-webkit transmission-qt ttf-bitstream-vera ttf-dejavu vlc webkitgtk xdg-user-dirs"

#echo "Remove GNOME/Cinnamon, Clear PKG Cache, Install KDE..."

do_uninstall() {
    echo "UNINSTALL"
#${remove2} || ${remove3} || ${remove4};
#${clean2} && ${install_kde} && kde_config
}

do_install() {
    echo "INSTALL"
#${remove2} || ${remove3} || ${remove4};
#${clean2} && ${install_kde} && kde_config
}



#kde_config() {

#echo "Configuring KDE.."
## Set KDE in .dmrc
#	echo "[Desktop]" >  /home/${1}/.dmrc
#	echo "Session=kde-plasma" >>  /home/${1}/.dmrc
#	chown ${1}:users	/home/${1}/.dmrc

#	# Download Flattr Icon Set
#    cd  /usr/share/icons
#    git clone https://github.com/NitruxSA/flattr-icons.git flattr-icons
#    cd flattr-icons
#    rm index.theme
#    mv index.theme.kde index.theme
#    sed -i 's|Example=x-directory-normal|Example=folder|g' index.theme
#    sed -i 's|Inherits=Flattr|Inherits=KFaenza,Oxygen|g' index.theme
#    rm -R .git
	
#	# Get zip file from github, unzip it and copy all setup files in their right places.
#	cd  /tmp
#    wget -q "https://github.com/Antergos/kde-setup/archive/master.zip"
#    unzip -o -qq  /tmp/master.zip -d  /tmp
#    cp -R  /tmp/kde-setup-master/etc  /
#    cp -R  /tmp/kde-setup-master/usr  /

#	# Set User & Root environments
#	cp -R  /etc/skel/.kde4  /home/${1}
#    cp -R  /etc/skel/.config  /root

#	## Set defaults directories
#	su -c xdg-user-dirs-update ${1}
	
#	# Fix Permissions
#	chown -R ${1}:users /home/${1}
#}


ZENITY=`which zenity`

if [ "$ZENITY" == "" ]; then
    pacman -S zenity --noconfirm
fi

ZENITY=`which zenity`

if [ "$ZENITY" == "" ]; then
    exit 0
fi

GNOME_SESSION=`which gnome-session`
CINNAMON_SESSION=`which cinnamon-session`
KDE_SESSION=`which startkde`

GNOME_INSTALLED="FALSE"
CINNAMON_INSTALLED="FALSE"
KDE_INSTALLED="FALSE"

if [ "$GNOME_SESSION" != "" ]; then
    GNOME_INSTALLED="TRUE"
fi

if [ "$CINNAMON_SESSION" != "" ]; then
    CINNAMON_INSTALLED="TRUE"
fi

if [ "$KDE_SESSION" != "" ]; then
    KDE_INSTALLED="TRUE"
fi

ANS=$(zenity --title="Antergos - Desktops" --height=300 --list  --text "Choose your desired desktops" --checklist  --column "Installed" --column "Desktop" $GNOME_INSTALLED "Gnome" $CINNAMON_INSTALLED "Cinnamon" $KDE_INSTALLED "KDE" --separator="|")

if [ "$ANS" == "" ]; then
    exit 0
fi

WANTS_GNOME="FALSE"
WANTS_CINNAMON="FALSE"
WANTS_KDE="FALSE"

INSTALL=""
UNINSTALL=""

for item in ${ANS//|/ }; do
    if [ $item == "Gnome" ]; then
        WANTS_GNOME="TRUE"
    elif [ $item == "Cinnamon" ]; then
        WANTS_CINNAMON="TRUE"
    elif [ $item == "KDE" ]; then
        WANTS_KDE="TRUE"
    fi
done

if [ "$WANTS_GNOME" == "TRUE" ]; then
    if [ "$GNOME_INSTALLED" == "FALSE" ]; then
        echo "Gnome will be installed"
        INSTALL=$INSTALL" GNOME"
    fi
else
    if [ "$GNOME_INSTALLED" == "TRUE" ]; then
        echo "Gnome will be uninstalled"
        UNINSTALL=$UNINSTALL" GNOME"
    fi
fi

if [ "$WANTS_CINNAMON" == "TRUE" ]; then
    if [ "$CINNAMON_INSTALLED" == "FALSE" ]; then
        echo "Cinnamon will be installed"
        INSTALL=$INSTALL" CINNAMON"
    fi
else
    if [ "$CINNAMON_INSTALLED" == "TRUE" ]; then
        echo "Cinnamon will be uninstalled"
        UNINSTALL=$UNINSTALL" CINNAMON"
    fi
fi

if [ "$WANTS_KDE" == "TRUE" ]; then
    if [ "$KDE_INSTALLED" == "FALSE" ]; then
        echo "KDE will be installed"
        INSTALL=$INSTALL" KDE"
    fi
else
    if [ "$KDE_INSTALLED" == "TRUE" ]; then
        echo "KDE will be uninstalled"
        UNINSTALL=$UNINSTALL" KDE"
    fi
fi

if [ "$INSTALL" == "" ] && [ "$UNINSTALL" == "" ]; then
    # Nothing to be done. Exit
    zenity --info --text="Nothing to be done. This script will end. Bye!"
    exit 0
fi

if ! zenity --question --text="Are you sure you wish to proceed?"; then
    exit 0
fi

for item in ${UNINSTALL// / }; do
    echo "UNINSTALLING $item"
    do_uninstall()
done

for item in ${INSTALL// / }; do
    echo "INSTALLING $item"
    do_install()
done

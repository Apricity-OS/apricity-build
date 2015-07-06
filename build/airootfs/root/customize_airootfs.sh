#!/bin/bash

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
#gpg --recv-keys 1EB2638FF56C0C53
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
#pacman -Syy

sed -i "s/#Server/Server/g" /etc/pacman.d/mirrorlist
sed -i 's/#\(Storage=\)auto/\1volatile/' /etc/systemd/journald.conf
echo 'seded'


systemctl enable graphical.target gdm.service pacman-init.service dhcpcd.service tlp.service tlp-sleep.service
systemctl -fq enable NetworkManager ModemManager
systemctl mask systemd-rfkill@.service
systemctl set-default graphical.target
touch /etc/pacman.d/antergos-mirrorlist
#echo "Server = http://repo.antergos.info/antergos/x86_64" > /etc/pacman.d/antergos-mirrorlist

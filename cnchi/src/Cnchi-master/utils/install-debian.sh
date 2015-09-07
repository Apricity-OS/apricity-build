#!/bin/bash
sudo apt-get -y install git glade geany
sudo apt-get -y install python3 python-cairo-dev python-gobject-dev python-dbus python3-mako python3-requests
sudo apt-get -y install hdparm upower parted dosfstools mtools ntfs-3g libtimezonemap1-dev gir1.2-timezonemap-1.0

wget "http://mirrors.kernel.org/ubuntu/pool/main/p/parted/libparted2_3.2-6_amd64.deb" -O /tmp/libparted2_3.2-6_amd64.deb
sudo dpkg -i /tmp/libparted2_3.2-6_amd64.deb
wget "http://mirrors.kernel.org/ubuntu/pool/universe/p/pyparted/python3-parted_3.10-1build1_amd64.deb" -O /tmp/python3-parted_3.10-1build1_amd64.deb
sudo dpkg -i /tmp/python3-parted_3.10-1build1_amd64.deb

# Not in Debian:
# python3-alpm (pyalpm)
# python3-webkit

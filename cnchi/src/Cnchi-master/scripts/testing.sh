#!/bin/bash

previous="/tmp/dev-setup"
uefi="/sys/firmware/efi"
vbox_chk="$(hwinfo --gfxcard | grep -o -m 1 "VirtualBox")"
arg="$1"
check_keys="$(pacman-key -l | grep Antergos)"

notify_user () {

       sudo -u antergos notify-send -t 10000 -a "Cnchi" -i /usr/share/cnchi/data/images/apricity/apricity-icon.png "$1"
}

do_update () {

# Update Cnchi with latest testing code
	echo "Removing existing Cnchi..."
	killall python
	mv  /usr/share/cnchi /usr/share/cnchi.old;
	if [[ ${development} = "True" ]]; then

		notify_user "Getting latest version of Cnchi from development branch..."
		echo "Getting latest version of Cnchi from development branch..."
		# Check commandline arguments to choose repo
		#if [ "$1" = "-d" ] || [ "$1" = "--dev-repo" ]; then
		#	git clone https://github.com/"$2"/Cnchi.git cnchi;
		#else
		#	git clone https://github.com/Antergos/Cnchi.git cnchi;
		#fi
		cd /tmp
		{ wget http://antergos.org/cnchi.tar && tar -xf cnchi.tar && cp -R cnchi /usr/share && rm cnchi.tar \
		 	&& rm -Rf cnchi && cd /usr/share/cnchi && return 0; } || \
		{ mv  /usr/share/cnchi.old /usr/share/cnchi && notify_user "Something went wrong. Update failed." \
		 && return 1; }
	else
		notify_user "Getting latest version of Cnchi from stable branch..."
		echo "Getting latest version of Cnchi from stable branch..."
		{ pacman -Syy --noconfirm cnchi && return 0; } || { mv  /usr/share/cnchi.old /usr/share/cnchi \
		&& notify_user "Something went wrong. Update failed." && return 1; }
	fi

}

start_cnchi () {

	# Start Cnchi with appropriate options
	notify_user "Starting Cnchi..."
	echo "Starting Cnchi..."
	if [[ ${development} = "True" ]]; then
		cnchi -d -v -p /usr/share/cnchi/data/packages.xml &
		exit 0;


	else

		cnchi -d -v &
		exit 0;

	fi

}

if [[ ${arg} = "development" ]]; then
	development=True
else
	stable=True
fi
# Check if this is the first time we are executed.
if ! [ -f "${previous}" ]; then
	touch ${previous};
	# Find the best mirrors (fastest and latest)
#    notify_user "Selecting the best mirrors..."
#	echo "Selecting the best mirrors..."
#	echo "Testing Arch mirrors..."
#	reflector -p http -l 30 -f 5 --save /etc/pacman.d/mirrorlist;
#	echo "Done."
#	sudo -u antergos wget http://antergos.info/antergos-mirrorlist
#	echo "Testing Antergos mirrors..."
#	rankmirrors -n 0 -r antergos antergos-mirrorlist > /tmp/antergos-mirrorlist
#	cp /tmp/antergos-mirrorlist /etc/pacman.d/
#	echo "Done."
	if [[ ${check_keys} = '' ]]; then
	pacman-key --init
	pacman-key --populate archlinux antergos
	fi

	# Install any packages that haven't been added to the iso yet but are needed.
	notify_user "Installing missing packages..."
	echo "Installing missing packages..."
	# Check if system is UEFI boot.
	if [ -d "${uefi}" ]; then
		pacman -Syy git efibootmgr --noconfirm --needed;
	else
		pacman -Syy git --noconfirm --needed;
	fi
	# Enable kernel modules and other services
	if [[ "${vbox_chk}" == "VirtualBox" ]] && [ -d "${uefi}" ]; then
		echo "VirtualBox detected. Checking kernel modules and starting services."
		modprobe -a vboxsf f2fs efivarfs dm-mod && systemctl restart vboxservice;
	elif [[ "${vbox_chk}" == "VirtualBox" ]]; then
		modprobe -a vboxsf f2fs dm-mod && systemctl restart vboxservice;
	else
		modprobe -a f2fs dm-mod;
	fi
	
else
    notify_user "Previous testing setup detected, skipping downloads..."
	echo "Previous testing setup detected, skipping downloads..."
	echo "Verifying that nothing is mounted from a previous install attempt."
	umount -lf /install/boot >/dev/null 2&>1
	umount -lf /install >/dev/null 2&>1

fi


do_update && start_cnchi


exit 1;

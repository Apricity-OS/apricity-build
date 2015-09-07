#! /bin/bash
set -e

# This script installs the GRUB2 theme in /boot/grub/themes/, /boot/grub2/themes/ or /grub/themes/
# depending on the distribution.
#
# Copyright (C) 2011 Towheed Mohammed
# who just started learning bash scripting, sed and regex's.
#
# This is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details at <http://www.gnu.org/licenses/>.

# Set variables
Theme_Name="Elegant_Dark_New"		# The theme will be installed in a dir with this name. Avoid spaces.
Theme_Definition_File="theme.txt"	# Filename of theme definition file.
Theme_Resolution="any"		# The resolution the theme was designed to show best at, 640x480, 1024x768 etc,
							# or "any" for any resolution (resolution independent).
Inst_Dir=$(dirname $0)
Grub_Dist_Dirs="/grub /boot/grub /boot/grub2"	# Directories must be in this order.
let Grub_Min_Version=198	# Do not change this.
Grub_File="/etc/default/grub"
Grub_Dir=
mkConfig_File=

# Check that the script is being run as root.
if [[ $(id -u) != 0 ]]; then
	echo "Please run this script with root privileges."
	exit 0
fi

# Get GRUB's installation directory.
for i in $Grub_Dist_Dirs; do
	if [[ -d $i ]]; then
		Grub_Dir=$i
	fi
done

# Exit this script if we could not locate GRUB's installation directory.
if [[ -z $Grub_Dir ]]; then
	echo "Could not locate GRUB's installation directory."
	exit 0
fi

# Exit the script if GRUB's version is < 1.98
if [[ -f $(which grub2-install) ]]; then
	Grub_Version_Long=$(grub2-install --version)
elif [[ -f $(which grub-install) ]]; then
	Grub_Version_Long=$(grub-install --version)
else
	echo 'Could not locate grub-install or grub2-install in your path.'
	exit 0
fi
Grub_Version=$(echo $Grub_Version_Long | sed 's,[[:alpha:][:punct:][:blank:]],,g')
if (( ${Grub_Version:0:3} < Grub_Min_Version )); then
	echo "GRUB must be at least version ${Grub_Min_Version:0:1}.${Grub_Min_Version:1:2}."
	echo "The installed version is ${Grub_Version:0:1}.${Grub_Version:1:2}."
	exit 0
fi

# Check that /etc/default/grub exists.
if [[ ! -f $Grub_File ]]; then
	echo "Could not find $Grub_File"
	exit 0
fi

# Check that GRUB's mkconfig script file exists.
mkConfig_File=$(which ${Grub_Dir##*/}-mkconfig) || \
(echo "GRUB's mkconfig script file was not found in your path." && exit 0)

# Create theme directory.  If directory already exists, ask the user if they would like
# to overwrite the contents with the new theme or create a new theme directory.
Theme_Dir=$Grub_Dir/themes/$Theme_Name
while [[ -d $Theme_Dir ]]; do
	echo "Directory $Theme_Dir already exists!"
	echo -n "Would you like to overwrite it's contents or create a new directory? [(o)verwrite (c)reate] "
	read Response
	case $Response in
		c|create)
			echo -n "Please enter a new name for the theme's directory: "
			read Response
			Theme_Dir=$Grub_Dir/themes/$Response;;
		o|overwrite)
			echo -n "This will delete all files in $Theme_Dir.  Are you sure? [(y)es (n)o] "
			read Response
			case $Response in
				y|yes)
					rm -r $Theme_Dir;;
				*)
					exit 0;;
			esac;;
		*)
			exit 0;;	
	esac
done
mkdir -p $Theme_Dir

# Copy the theme's files to the theme's directory.
for i in $Inst_Dir/*; do
	cp -r $i $Theme_Dir/$(basename $i)
done

# Check whether an icons directory exists.  If icons are not included in this theme,
# check if one exists in ..../themes/icons.  If it exists, ask the user if they would like to use it.
if [[ ! -d $Theme_Dir/icons && -d $Grub_Dir/themes/icons ]]; then
	echo "An icons directory was not included in this theme."
	echo "However, one was found in $Grub_Dir/themes/icons containing these files:"
	echo $(ls $Grub_Dir/themes/icons)
	echo -n "Would you like to use these icons? [(y)es (n)o] "
	read Response
	case $Response in
		y|yes)
			ln -s $Grub_Dir/themes/icons $Theme_Dir/;;
		*)
			echo "This theme will not show any icons.";;
	esac
elif [[ ! $Theme_Dir/icons && ! -d $Grub_Dir/themes/icons ]]; then
	echo "Could not find an icons directory.  This theme will not show any icons."
fi

# Change GRUB's resolution to match that of the theme.
if [[ $Theme_Resolution != "any" ]]; then
	i=$(sed -n 's,^#\?GRUB_GFXMODE=,&,p' $Grub_File)
	if [[ -z $i ]]; then
		echo -e "\nGRUB_GFXMODE=$Theme_Resolution" >>$Grub_File
	else
		sed "s,^#\?GRUB_GFXMODE=.*,GRUB_GFXMODE=$Theme_Resolution," <$Grub_File >$Grub_File.~
		mv $Grub_File.~ $Grub_File
	fi
fi

# Ask the user if they would like to set the theme as their new theme.

if [[ yes = yes || y = y ]]; then
	i=$(sed -n 's,^#\?GRUB_THEME=,&,p' $Grub_File)
	if [[ -z $i ]]; then
		echo -e "\nGRUB_THEME=$Theme_Dir/$Theme_Definition_File" >>$Grub_File
	else
		sed "s,^#\?GRUB_THEME=.*,GRUB_THEME=$Theme_Dir/$Theme_Definition_File," <$Grub_File >$Grub_File.~
		mv $Grub_File.~ $Grub_File
	fi	
	$($mkConfig_File -o $Grub_Dir/grub.cfg)	# Generate new grub.cfg
fi
exit 0

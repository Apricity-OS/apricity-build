#!/bin/sh
kernel_cmdline ()
{
    for param in $(/bin/cat /proc/cmdline); do
        case "${param}" in
            $1=*) echo "${param##*=}"; return 0 ;;
            $1) return 0 ;;
            *) continue ;;
        esac
    done
    [ -n "${2}" ] && echo "${2}"
    return 1
}

USENONFREE="$(kernel_cmdline nonfree no)"
VIDEO="$(kernel_cmdline xdriver no)"
DESTDIR="$1"

echo "MHWD-Driver: ${USENONFREE}"
echo "MHWD-Video: ${VIDEO}"

mkdir -p ${DESTDIR}/opt/livecd
mount -o bind /opt/livecd ${DESTDIR}/opt/livecd > /tmp/mount.pkgs.log
ls ${DESTDIR}/opt/livecd >> /tmp/mount.pkgs.log

# Video driver
if  [ "${USENONFREE}" == "yes" ] || [ "${USENONFREE}" == "true" ]; then
	if  [ "${VIDEO}" == "vesa" ]; then
		chroot ${DESTDIR} mhwd --install pci video-vesa --pmconfig "/opt/livecd/pacman-gfx.conf" 
	else
		chroot ${DESTDIR} mhwd --auto pci nonfree 0300 --pmconfig "/opt/livecd/pacman-gfx.conf" 
	fi
else
	if  [ "${VIDEO}" == "vesa" ]; then
		chroot ${DESTDIR} mhwd --install pci video-vesa --pmconfig "/opt/livecd/pacman-gfx.conf" 
	else
		chroot ${DESTDIR} mhwd --auto pci free 0300 --pmconfig "/opt/livecd/pacman-gfx.conf" 
	fi
fi

# Network driver
mhwd --auto pci free 0200 --pmconfig "/opt/livecd/pacman-gfx.conf"
mhwd --auto pci free 0280 --pmconfig "/opt/livecd/pacman-gfx.conf"

umount ${DESTDIR}/opt/livecd
rmdir ${DESTDIR}/opt/livecd

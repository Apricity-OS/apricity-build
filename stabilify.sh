sed -i -e 's/\[apricity-core-dev\]/\[apricity-core\]/g' pacman/pacman.x86_64.conf
sed -i -e 's/apricity-core-dev/apricity-core-signed/g' pacman/pacman.x86_64.conf
sed -i -e 's/\[apricity-core-dev\]/\[apricity-core\]/g' airootfs-all/etc/pacman.conf
sed -i -e 's/apricity-core-dev/apricity-core-signed/g' airootfs-all/etc/pacman.conf

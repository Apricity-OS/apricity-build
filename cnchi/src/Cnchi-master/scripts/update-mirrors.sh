#!/bin/bash

# Reflector actually does perform speed tests when called and doesn't use speed results
# from Arch's main server like we originally thought. If you monitor your network I/O
# while running reflector command, you will see it is doing much more than
# downloading the 4kb mirrorlist file. If ran with --verbose and without --save param it
# will output the speed test results to STDOUT.

if [ -f /usr/bin/reflector ]; then
    reflector -l 30 -p http -f 20 --save /etc/pacman.d/mirrorlist
fi

# Remember that running rankmirrors takes time. We use a .new file to avoid getting an empty antergos-mirrorlist
# if the user exists before rankmirrors can finish
if [ -f /usr/bin/rankmirrors ]; then
    rankmirrors -n 0 -r antergos /etc/pacman.d/antergos-mirrorlist > /etc/pacman.d/antergos-mirrorlist.new
    cp /etc/pacman.d/antergos-mirrorlist /etc/pacman.d/antergos-mirrorlist.old
    cp /etc/pacman.d/antergos-mirrorlist.new /etc/pacman.d/antergos-mirrorlist
fi

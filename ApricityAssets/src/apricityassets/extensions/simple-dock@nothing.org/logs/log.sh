#!/bin/bash

# Call: imports.misc.util.spawnCommandLine('/home/albert/.local/share/gnome-shell/extensions/simple-dock@nothing.org/logs/log.sh si');
# Logs: tail -f ~/.local/share/gnome-shell/extensions/simple-dock@nothing.org/logs/log.tmp
echo $1 >> /home/albert/.local/share/gnome-shell/extensions/simple-dock@nothing.org/logs/log.tmp


userdel liveuser
rm /etc/gdm/custom.conf
USER=$(cat /etc/passwd |grep "/home" |cut -d: -f1)
usermod -s /bin/zsh $USER

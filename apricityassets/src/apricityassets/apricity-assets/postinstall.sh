os-prober && grub-mkconfig -o /boot/grub/grub.cfg
USER=$(cat /etc/passwd |grep "/home" |cut -d: -f1)
usermod -s /bin/zsh $USER

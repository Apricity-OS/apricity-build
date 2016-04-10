#TODO

####Bugs
- [ ] Fix [bug with numix-folders](https://github.com/numixproject/numix-folders/issues/133)
- [ ] Fix all the GTK warnings about some parts of the icon theme not declaring a size field
- [ ] Chrome extensions enabled by default may be broken on some machines
- [ ] Issue with steam [thread1](https://github.com/Apricity-OS/apricity-build/issues/20#issuecomment-206939955), [thread2](https://github.com/Apricity-OS/apricity-build/issues/20#issuecomment-206939955)

####Distro Features
- [ ] Add a 32-bit version (shouldn't be too hard, just need to test on a 32-bit machine)
- [ ] Add a more verbose error to the installer when a user decides to install a bootloader on a UEFI machine and chooses manual partitioning but forgets to create a FAT32-formatted /boot/efi partition
- [ ] Automatic timezone selection for the installer
- [ ] Replace (or augment) `simple-backup` with [timeshift](http://www.teejeetech.in/p/timeshift.html)?
- [ ] Replace `gnome-terminal` with [terminix](https://github.com/gnunn1/terminix)
- [ ] Update to `calamares-2.2`
- [ ] Replace Pushbullet with [KDEConnect](https://community.kde.org/KDEConnect) or [Pushjet](https://pushjet.io/)
- [ ] Replace Pamac with something? It's a little buggy.

####General Features
- [ ] Automatically update PATRONS.md weekly with [patreon-api](https://github.com/oxguy3/patreon-api)]?
- [ ] Automatically update `calamares` weekly?
- [ ] Enable package signing in repo
- [ ] Automatically rebuild repo daily, pulling from the AUR and from github for Apricity packages (eventially this could be run in an AWS EC2 spot instance)
- [ ] Automatically build dev ISO in an AWS spot instance bi-weekly
- [ ] Split github repos into stable and dev
- [ ] Add sha256 tar hashes to all Apricity packages
- [ ] Update all PKGBUILD urls to reflect the move to [github.com/Apricity-OS](https://github.com/Apricity-OS)
- [ ] Add READMEs to all repositories
- [ ] Replace `apricityassets`. It's a little confusing and convoluted. Replace it with `apricity-assets`, `apricity-plymouth`, and `apricity-gnome-extensions`.
- [ ] Automatically update extensions from github

####Desktop Environments
- [x] Gnome
- [x] Cinnamon
- [ ] XFCE
- [ ] KDE
- [ ] i3 / [sway](http://swaywm.org/)?
- [ ] Budgie?
- [ ] Pantheon?

####General
- [ ] Write docs for build scripts and installation
- [ ] Add badges to the forum?

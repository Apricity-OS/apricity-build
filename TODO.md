#TODO

####Bugs
- [x] Fix [bug with numix-folders](https://github.com/numixproject/numix-folders/issues/133)
- [x] Fix all the GTK warnings about some parts of the icon theme not declaring a size field

####Distro Features
- [x] Remove support a 32-bit version (shouldn't be too hard, just need to test on a 32-bit machine)
- [ ] Add a more verbose error to the installer when a user decides to install a bootloader on a UEFI machine and chooses manual partitioning but forgets to create a FAT32-formatted /boot/efi partition
- [ ] Automatic timezone selection for the installer
- [ ] Replace (or augment) `simple-backup` with [timeshift](http://www.teejeetech.in/p/timeshift.html)?
- [ ] Replace `gnome-terminal` with [terminix](https://github.com/gnunn1/terminix)
- [ ] Update to `calamares-2.2`
- [ ] Replace Pushbullet with [KDEConnect](https://community.kde.org/KDEConnect) or [Pushjet](https://pushjet.io/)
- [ ] Replace Pamac with something? It's a little buggy.
- [x] Updates should update the distro version
- [ ] Add a dark theme with lighter symbolic icons
- [ ] Think about replacing GDM with SDDM or LightDM
- [ ] Calamares LUKS support
- [ ] Switch to vivaldi from chrome?
- [ ] Find an easy tool for users to switch kernel versions
- [ ] Find an easy tool for users to switch to proprietary graphics drivers
- [x] Add `cronie` to package list
- [x] Remove `gtk-theme-murrine-arch` from `apricity-core`
- [ ] Improve the Plymouth theme (and make it work on UEFI installs)
- [ ] Add either unite.vim or fzf.vim
- [x] Add pacaur
- [x] Switch to xf86-input-libinput
- [ ] Fix autologin bug for existing installs
- [ ] Switch to signed repo for existing installs
- [ ] More elegant firstrun.sh functionality

####General Features
- [ ] Automatically update `calamares` weekly?
- [ ] Enable package signing in repo
- [ ] Automatically build dev ISO in an AWS spot instance bi-weekly
- [x] Split github repos into stable and dev
- [ ] Add READMEs to all repositories
- [ ] Replace `condresassets`. It's a little confusing and convoluted. Replace it with `condres-assets`, `condres-plymouth`, and `condres-gnome-extensions`.
- [ ] Automatically update extensions from github
- [x] Remove mirrors to `apricity-core`
- [ ] **Custom ISO Builder**
  - This is going to be a web form that one can fill out to select a DE, their preferred applications, a shell and GUI theme, an icon theme, some wallpapers, `systemd` services to enable, and a branch of the kernel to install. Then a custom ISO would get built on AWS and a link would be emailed to the user. The *issue* is that this would cost roughly $0.10 per build, so we need some sort of increase in revenue.
- [ ] **Cloud Configuration**
  - This is a tool/service to sync installed applications, conf files, dotfiles, and anything else the user specifies across machines, and perhaps also to save backups in the cloud.
  - This could potentially source the revenue needed to offer the above custom ISO builder for free.
- [ ] **Update Wikipedia page**
- [ ] Auto build monthly torrent for snapshots
- [ ] Add pithos, yandex, vivaldi, and spotify to the repos

####Desktop Environments
- [x] Gnome
- [x] Cinnamon
- [x] XFCE
- [x] KDE
- [x] MATE
- [ ] i3 / [sway](http://swaywm.org/)?
- [x] Budgie?
- [x] Pantheon?

####General
- [ ] Write docs for build scripts and installation
- [ ] Add badges to the forum?

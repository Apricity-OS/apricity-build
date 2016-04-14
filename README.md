# Apricity OS

**Thinking about contributing? Click below to take a look at what developers are talking about on Gitter.**

[![Join the chat at https://gitter.im/Apricity-OS/apricity-build](https://badges.gitter.im/Apricity-OS/apricity-build.svg)](https://gitter.im/Apricity-OS/apricity-build?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

####Links
Download The March 64-Bit GNOME Version [Direct](https://sourceforge.net/projects/apricityos/files/apricity_os-gnome-03.2016-beta.iso/download) or [Torrent](http://apricityos.com/iso/apricity_os-gnome-03.2016-beta.torrent)

Download The March 64-Bit Cinnamon Version [Direct](https://sourceforge.net/projects/apricityos/files/apricity_os-cinnamon-03.2016-beta.iso/download) or [Torrent](http://apricityos.com/iso/apricity_os-cinnamon-03.2016-beta.torrent)

Support Apricity on [Patreon](http://www.patreon.com/apricity)

![Apricity Laptop](http://apricityos.com/assets/img/back/apricity-laptop.jpg)

####Apricity's Goal: Simple, Beautiful Linux

####What's Apricity OS
Apricity OS is an Arch Linux-based distribution with a nice-looking default interface, useful preinstalled applications, and two editions (more coming soon!). The idea is that if your Linux distribution works well as soon as you install it, you have to do less work. If you then decide to install Apricity on another machine, everything still works.

####What's configured out of the box?
**Desktop Environment (DE) Theme:** This is mainly the Gnome or Cinnamon top or bottom panel, respectively. For Gnome, it also includes how the activities' overview, workspace switching, and the dock look. For Cinnamon, this also includes workspace switching, the application menu, and the window overview. The Gnome shell theme is custom built for Apricity, and the Cinnamon theme is based on Arc.

**GTK Theme:** Apricity uses [Arc GTK](https://github.com/horst3180/arc-theme) by default, but post something on [Gitter](https://gitter.im/Apricity-OS/apricity-build) if you know of other great-looking GTK themes.

**DE Extensions:** Gnome is a little unintuitive and clunky to use normally, but the Gnome developers have made it easy for other developers to customize its functionality with extensions. The extensions that are enabled by default in Apricity are as follows:
- Caffeine
- Dash to dock
- Frippery move clock
- Media player indicator
- Places status indicator
- Removable drive menu
- Remove dropdown arrows
- Suspend button
- Top panel workspace scroll
- Topicons
- User themes

**Icons:** Application icons come by default as `numix-circle`, and the various symbolic and mimetype icons are a combination of `vimix` and `paper`.

**Terminal:** Apricity uses `zsh` and `powerline-shell` in its default terminal.

**Browser:** This is a point of some debate. Several people have brought up the point that Chromium is available as an open source alternative to Chrome, which is the current default. The counter-argument is that Chrome has better support for closed-source codecs. The decision to include a set of Chrome extensions, enumerated below, was also somewhat contraversial. **Update: default extensions may be broken in the latest release?** For further discussion, refer to [Gitter](https://gitter.im/Apricity-OS/apricity-build), or feel free to start a thread on the [forum](http://apricityos.com/forum) or an [issue](https://github.com/Apricity-OS/apricity-build/issues).
- Pushbullet (may be replaced by [KDEConnect](https://community.kde.org/KDEConnect) or [Pushjet](https://pushjet.io/) in the near future)
- Adblock Plus
- Ghostery
- The Great Suspender

**Wallpapers:** Apricity comes with a really nice set of [Creative Commons](https://creativecommons.org/) wallpapers, some of which go really well with the default Apricity themes, and some of which just look great on their own. If you have any suggestions for wallpapers to include in future releases, either post something on [Gitter](https://gitter.im/Apricity-OS/apricity-build), the [forum](http://apricityos.com/forum), or just submit a pull request to the [apricity-wallpapers](https://github.com/Apricity-OS/apricity-wallpapers) repository.

**Vim:** The following plugins are included in Apricity's default Vim configuration:
- Pathogen
- Nerdtree
- Vim-nerdtree-tabs
- Syntastic
- Vim-airline
- Vim-sensible
- Vim-tabber

Also of note: Apricity's Vim uses a slightly modified version of the [Monokai](https://github.com/sickill/vim-monokai) colorscheme.

####What applications are included in Apricity?
Here's an (almost exhaustive) list of included programs. Please feel free to discuss these choices on [Gitter](https://gitter.im/Apricity-OS/apricity-build) or the [forum](http://apricityos.com/forum).
- Pamac (Add/Remove Software; Software Update)
- File Roller (Archive Manager)
- Gnome-Calculator
- Gnome-Calendar
- Cheese
- Gnome-Disks
- Baobab (Disk Usage Analyzer)
- Evince (Document Viewer)
- Nautilus / Nemo (Files)
- FileZilla
- Uncomplicated Firewall (Firewall Configuration)
- Font Viewer
- Gedit
- GIMP
- Google Chrome
- Ice (Peppermint's Site Specific Browser Tool)
- Eye of Gnome (Image Viewer)
- Inkscape
- LibreOffice
- Gnome-Photos
- PlayOnLinux
- Rhythmbox
- Orca (Screen Reader)
- Gnome-Screenshot
- Settings
- Simple Backup
- Steam **[Sometimes Broken?](https://github.com/Apricity-OS/apricity-build/issues/20#issuecomment-206939955), [Also Here](http://apricityos.com/forum/discussion/comment/1114#Comment_1114)**
- Syncthing
- System Log
- System Monitor
- Gnome-Terminal
- Transmission
- Tweak Tool (Only on Gnome)
- Totem (Gnome-Videos)

####So you want to contribute?
That's great! Fork *this* repository, pull it to your local machine, and start by building a version of the ISO. You'll need to be running something Arch-based, since you need to install `archiso` to run the build scripts. Then run `su -c './build.sh -v -E gnome -R true'`, type your root password, and watch as `mkarchiso` does its magic. (If I forgot to put a dependency here, please create an [issue](https://github.com/Apricity-OS/apricity-build/issues))

Once you've built an image, you can test it with `gnome-boxes`, `virtualbox`, or another emulator of your choice. Now, as a quick experiment, try opening `apricity-build/packages/packages.all.x86_64` and adding or removing something. Play with the build script and get comfortable using it. See if you can figure out what the other command-line arguments are, and try to build the Cinnamon edition. (Official docs are coming soon, I promise!)

Now pick something to work on. Take a look at [TODO.md](https://github.com/Apricity-OS/apricity-build/blob/master/TODO.md) for ideas. Once you've finished your feature or fixed your bug, submit a [pull request](https://help.github.com/articles/using-pull-requests/). If everything looks good, it will get incorporated first into the `dev` branch, then later into the `stable` branch!

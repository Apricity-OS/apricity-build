# Condres OS

**Thinking about contributing? Click below to take a look at what developers are talking about on Gitter.**



SupportCondres on (https://www.codelinsoft.it/sito/2015-02-18-23-59-25/donate.html)



####Condres's Goal: Simple, Beautiful Linux

####What's Condres OS
Condres OS is an Arch Linux-based distribution with a nice-looking default interface, useful preinstalled applications, and two editions (more coming soon!). The idea is that if your Linux distribution works well as soon as you install it, you have to do less work. If you then decide to installCondres on another machine, everything still works.

####What's configured out of the box?
**Desktop Environment (DE) Theme:** This is mainly the Gnome or Cinnamon top or bottom panel, respectively. For Gnome, it also includes how the activities' overview, workspace switching, and the dock look. For Cinnamon, this also includes workspace switching, the application menu, and the window overview. The Gnome shell theme is custom built forCondres, and the Cinnamon theme is based on Arc.

**GTK Theme:**Condres uses [Arc GTK](https://github.com/horst3180/arc-theme) by default, but post something on [Gitter](https://gitter.im/Condres-OS/Condres-build) if you know of other great-looking GTK themes.

**DE Extensions:** Gnome is a little unintuitive and clunky to use normally, but the Gnome developers have made it easy for other developers to customize its functionality with extensions. The extensions that are enabled by default inCondres are as follows:
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

**Terminal:**Condres uses `zsh` and `powerline-shell` in its default terminal.



Also of note:Condres's Vim uses a slightly modified version of the [Monokai](https://github.com/sickill/vim-monokai) colorscheme.

####What applications are included inCondres?

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
- Syncthing
- System Log
- System Monitor
- Gnome-Terminal
- Transmission
- Tweak Tool (Only on Gnome)
- Totem (Gnome-Videos)

####So you want to contribute?
That's great! Fork *this* repository, pull it to your local machine, and start by building a version of the ISO. You'll need to be running something Arch-based, since you need to install `archiso` to run the build scripts. Then run `su -c './build.sh -v -E gnome -R true'`, type your root password, and watch as `mkarchiso` does its magic. (If I forgot to put a dependency here, please create an [issue](https://github.com/Condres-OS/Condres-build/issues))

Once you've built an image, you can test it with `gnome-boxes`, `virtualbox`, or another emulator of your choice. Now, as a quick experiment, try opening `Condres-build/packages/packages.all.x86_64` and adding or removing something. Play with the build script and get comfortable using it. See if you can figure out what the other command-line arguments are, and try to build the Cinnamon edition. (Official docs are coming soon, I promise!)

Now pick something to work on. Take a look at [TODO.md] for ideas. Once you've finished your feature or fixed your bug, submit a [pull request](https://help.github.com/articles/using-pull-requests/). If everything looks good, it will get incorporated first into the `dev` branch, then later into the `stable` branch!

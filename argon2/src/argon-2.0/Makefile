SHELL = /bin/bash
DESTDIR       =
INSTALL = /usr//bin/install -c
bindir = /usr/bin
deskdir = /usr/share/applications
icondir = /usr/share/argon
configdir = /etc/argon

all:

install: all
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) -m755 argon $(DESTDIR)$(bindir)
	$(INSTALL) -m755 argon-notifier $(DESTDIR)$(bindir)
	$(INSTALL) -m755 argon-notifier-config $(DESTDIR)$(bindir)
	$(INSTALL) -m755 argon-install $(DESTDIR)$(bindir)
	$(INSTALL) -m755 argon-remove $(DESTDIR)$(bindir)
	$(INSTALL) -m755 argon-list $(DESTDIR)$(bindir)
	$(INSTALL) -m755 argon-update $(DESTDIR)$(bindir)
	$(INSTALL) -d $(DESTDIR)$(deskdir)
	$(INSTALL) -m644 argon.desktop $(DESTDIR)$(deskdir)
	$(INSTALL) -m644 argon-notifier-config.desktop $(DESTDIR)$(deskdir)
	mkdir -p $(DESTDIR)$(icondir)
	$(INSTALL) -m644 argon-update-notifier.desktop $(DESTDIR)$(icondir)
	$(INSTALL) -m644 argon.png $(DESTDIR)$(icondir)
	$(INSTALL) -m644 arch-fresh.png $(DESTDIR)$(icondir)
	$(INSTALL) -m644 arch-stale.png $(DESTDIR)$(icondir)
	mkdir -p $(DESTDIR)$(configdir)
	$(INSTALL) -m666 config $(DESTDIR)$(configdir)
	$(INSTALL) -m666 package $(DESTDIR)$(configdir)
	$(INSTALL) -m666 message $(DESTDIR)$(configdir)
	$(INSTALL) -m666 top-final $(DESTDIR)$(configdir)
	$(INSTALL) -m666 top-add $(DESTDIR)$(configdir)
	$(INSTALL) -m666 top-remove $(DESTDIR)$(configdir)

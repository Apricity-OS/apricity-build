#!/usr/bin/python

from gi.repository import Gtk, TimezoneMap

def on_timezone_changed(tz_map, timezone):
	print(timezone)

def on_destroy(window):
	Gtk.main_quit()

if __name__ == "__main__":
	window = Gtk.Window()
	tz_map = TimezoneMap.TimezoneMap()

	window.connect("destroy", on_destroy)
	tz_map.connect('location-changed', on_timezone_changed)

	window.add(tz_map)
	window.show_all()

	tz_map.set_timezone("Europe/Prague")
	Gtk.main()

from gi.repository import Gtk
import gi.types
import types

def do_recurse(obj, base_name):
    #print "   start recurse of", obj, base_name
    for i in dir(obj):
        if i.startswith("__") and i.endswith("__"):
            continue

        try:
            attr = getattr(obj, i)
        except NotImplementedError:
            continue
        abs_name = base_name + "." + i
        #print "   ", i, type(attr)

        if isinstance(attr, int):
            print(abs_name, "(integer constant)")

        if isinstance(attr, types.MethodType):
            print(abs_name + "() (instance method)")

        if isinstance(attr, types.FunctionType):
            print(abs_name + "() (function)")

        if isinstance(attr, type):
            do_recurse(attr, abs_name)

def main():
    do_recurse(Gtk, "Gtk")

if __name__ == "__main__":
    main()


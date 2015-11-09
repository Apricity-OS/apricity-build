#!/usr/bin/env python3
#
# by Kendall Weaver <kendall@peppermintos.com>
# for Peppermint OS
#
# Ice is a simple Site Specific Browser (SSB) manager for Chromium and
# Chrome specifically intended to integrate with the LXDE menu system.
# Unlike the built-in functions in the browsers, Ice boasts the ability
# to remove SSBs, validate addresses, and prevent overwriting existing
# SSBs. Special thanks to Matt Phillips <labratmatt@gmail.com> for the
# excellent pyfav library that is integrated into this application.

import os, sys, requests
import urllib.request, urllib.parse, urllib.error, os.path, string
from gi.repository import Gtk
from gi.repository.GdkPixbuf import Pixbuf
from urllib.parse import urlparse, urlunparse
from bs4 import BeautifulSoup

if not os.path.exists(os.path.expanduser('~/.local/share/applications/')):
    os.system("mkdir -p $HOME/.local/share/applications")

os.system("mkdir -p ~/.local/share/ice")

headers = {
    'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_2) \
        AppleWebKit/537.36 (KHTML, like Gecko) Chrome/33.0.1750.152 \
        Safari/537.36'
}

def get_details(app):
    a = open(app, 'r')
    nameline = ""
    iconline = ""
    is_ice = False
    for line in a:
        if "Name=" in line:
            array = line.replace("=", " ").split()
            array.pop(0)
            for word in array:
                nameline = nameline + word + " "
        elif "Icon=" in line:
            array = line.replace("=", " ").split()
            array.pop(0)
            for word in array:
                iconline = iconline + word
            try:
                pixbuf = Pixbuf.new_from_file_at_size(iconline, 16, 16)
            except:
                pixbuf = Pixbuf.new_from_file_at_size("/usr/share/pixmaps/ice.png", 16, 16)
        elif "StartupWMClass=Chromium" in line:
            is_ice = True
            
    if not nameline == None and not iconline == None and is_ice == True:
        return (nameline, pixbuf)
    else:
        return (None, None)

def normalize(url):
    (scheme, netloc, path, _, _, frag) = urlparse(url, "http")
    if not netloc and path:
        return urlunparse((scheme, path, "", "", "", ""))
    else:
        return urlunparse((scheme, netloc, path, "", "", ""))

def errortest(url):
    try:
        urllib.request.urlopen(url)
        return True
    except urllib.request.HTTPError:
        return False
    except urllib.request.URLError:
        return False
    return None

def download_favicon(url, file_prefix='', target_dir='/tmp'):
    parsed_site_uri = urlparse(url)

    if not parsed_site_uri.scheme:
        url = 'http://' + url
        parsed_site_uri = urlparse(url)

    if not parsed_site_uri.scheme or not parsed_site_uri.netloc:
        raise Exception("Unable to parse URL, %s" % url)

    favicon_url = get_favicon_url(url)

    if not favicon_url:
        raise Exception("Unable to find favicon for, %s" % url)

    response = requests.get(favicon_url, headers=headers)
    if response.status_code == requests.codes.ok:
        parsed_uri = urlparse(favicon_url)
        favicon_filepath = parsed_uri.path
        favicon_path, favicon_filename  = os.path.split(favicon_filepath)

    valid_chars = "-_.() %s%s" % (string.ascii_letters, string.digits)
    
    sanitized_filename = "".join([x if valid_chars \
        else "" for x in favicon_filename])
        
    sanitized_filename = os.path.join(target_dir, file_prefix + 
        sanitized_filename)
        
    with open(sanitized_filename, 'wb') as f:
            for chunk in response.iter_content(chunk_size=1024): 
                if chunk: # filter out keep-alive new chunks
                    f.write(chunk)
                    f.flush()
                    
    return sanitized_filename

def parse_markup_for_favicon(markup, url):
    parsed_site_uri = urlparse(url)
    
    soup = BeautifulSoup(markup)
        
    icon_link = soup.find('link', rel='icon')
    if icon_link and icon_link.has_attr('href'):
        
        favicon_url = icon_link['href']
        
        if favicon_url.startswith('//'):
            parsed_uri = urlparse(url)
            favicon_url = parsed_uri.scheme + ':' + favicon_url

        elif favicon_url.startswith('/'):
            favicon_url = parsed_site_uri.scheme + '://' + \
                parsed_site_uri.netloc + favicon_url
        
        elif not favicon_url.startswith('http'):
            path, filename  = os.path.split(parsed_site_uri.path)
            favicon_url = parsed_site_uri.scheme + '://' + \
                parsed_site_uri.netloc + '/' + os.path.join(path, favicon_url)
        
        return favicon_url

    return None

def get_favicon_url(url):
    parsed_site_uri = urlparse(url)

    try:
        response = requests.get(url, headers=headers)
    except:
        raise Exception("Unable to find URL. Is it valid? %s" % url)
    
    if response.status_code == requests.codes.ok:
        favicon_url = parse_markup_for_favicon(response.content, url)

        if favicon_url:
            return favicon_url
            
    favicon_url = '{uri.scheme}://{uri.netloc}/favicon.ico'.format(\
        uri=parsed_site_uri)
                        
    response = requests.get(favicon_url, headers=headers)
    if response.status_code == requests.codes.ok:
        return favicon_url

    return None

def applicate():
    title = name.get_text()
    address = normalize(url.get_text())
    
    semiformatted = ""
    array = filter(str.isalpha, title)
    for obj in array:
        semiformatted = semiformatted + obj
    formatted = semiformatted.lower()
    
    loc = where.get_active_text()
    if loc == "Accessories":
        location = "Utility;"
    elif loc == "Games":
        location = "Game;"
    elif loc == "Graphics":
        location = "Graphics;"
    elif loc == "Internet":
        location = "Network;"
    elif loc == "Office":
        location = "Office;"
    elif loc == "Programming":
        location = "Development;"
    elif loc == "Sound & Video":
        location = "AudioVideo;"
    elif loc == "System Tools":
        location = "System;"
            
    global iconpath
    iconname = iconpath.replace("/", " ").split()[-1]
    iconext = iconname.replace(".", " ").split()[-1]
            
    if os.path.exists(os.path.expanduser("~/.local/share/applications/" + formatted + ".desktop")):
        DuplicateError(title, formatted, address, iconext, location)
    else:
        writefile(title, formatted, address, iconext, location)

def writefile(title, formatted, address, iconext, location):
    global iconpath
    os.system("cp --force " + iconpath + " $HOME/.local/share/ice/" + formatted + "." + iconext)
    appfile = os.path.expanduser("~/.local/share/applications/" + formatted + ".desktop")
    os.system("touch " + appfile)
    if chrome.get_active() == True:
        browser = "google-chrome"
    elif chromium.get_active() == True:
        browser = "chromium-browser"
    elif firefox.get_active() == True:
        browser = "ice-firefox"
    else:
        print("ERROR: An unknown browser selection error has occurred.")
        sys.exit(1)

    with open(appfile, 'w') as appfile1:
        appfile1.truncate()
        appfile1.write("[Desktop Entry]\n")
        appfile1.write("Version=1.0\n")
        appfile1.write("Name=" + title + "\n")
        if (browser == "ice-firefox"):
            appfile1.write("Exec=" + browser + " " + address + "\n")
        else:
            appfile1.write("Exec=" + browser + " --app=" + address + "\n")
        appfile1.write("Terminal=false\n")
        appfile1.write("X-MultipleArgs=false\n")
        appfile1.write("Type=Application\n")
        appfile1.write("Icon=" + os.path.expanduser("~/.local/share/ice/") + formatted + "." + iconext + "\n")
        appfile1.write("Categories=GTK;" + location + "\n")
        appfile1.write("MimeType=text/html;text/xml;application/xhtml_xml;\n")
        appfile1.write("StartupWMClass=Chromium\n")
        appfile1.write("StartupNotify=true\n")
        if (browser == "ice-firefox"):
            address1 = str.replace(address, 'http://', '')
            appfile1.write("IceFirefox=" + str.replace(address1, 'https://', ''))
            
    name.set_text("")
    url.set_text("")
    iconpath = "/usr/share/pixmaps/ice.png"
    new_icon = Pixbuf.new_from_file_at_size(iconpath, 32, 32)
    icon.set_from_pixbuf(new_icon)
    iconline, pixbuf = get_details(appfile)
    liststore.prepend([pixbuf, iconline])
    
def delete(button):
    a = iconview.get_selected_items()
    b = liststore.get_iter(a[0])
    c = liststore.get_value(b, 1)
    liststore.remove(b)
    
    semiformatted = ""
    array = filter(str.isalpha, c)
    for obj in array:
        semiformatted = semiformatted + obj
    formatted = semiformatted.lower()
    appfile = os.path.expanduser("~/.local/share/applications/" + formatted + ".desktop")

    appfileopen = open(appfile, 'r')
    appfilelines = appfileopen.readlines()
    appfileopen.close()

    for line in appfilelines:
        if "IceFirefox=" in line:
            profile = str.replace(line, 'IceFirefox=', '')
            directory = os.path.expanduser('~/.local/share/ice/firefox')

            for profiles in os.listdir(directory):
                if profile in profiles:
                    os.system("rm -rf " + directory + "/" + profile)

    os.system("rm " + appfile)

class IconSel(Gtk.FileChooserDialog):

    def __init__(self):

        def update_image(dialog):
            filename = dialog.get_preview_filename()
            
            try:
                pixbuf = Pixbuf.new_from_file(filename)
                preview.set_from_pixbuf(pixbuf)
                valid_preview = True
            except:
                valid_preview = False
                
            dialog.set_preview_widget_active(valid_preview)

        filew = Gtk.FileChooserDialog("Please choose an icon.", None, Gtk.FileChooserAction.OPEN, (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OPEN, Gtk.ResponseType.OK))
        filew.set_filename("/usr/share/pixmaps/ice.png")

        filter1 = Gtk.FileFilter()
        filter1.set_name("Icons")
        filter1.add_mime_type("image/png")
        filter1.add_mime_type("image/jpeg")
        filter1.add_mime_type("image/gif")
        filter1.add_pattern("*.png")
        filter1.add_pattern("*.jpg")
        filter1.add_pattern("*.gif")
        filter1.add_pattern("*.xpm")
        filter1.add_pattern("*.svg")
        filew.add_filter(filter1)
        
        preview = Gtk.Image()
        filew.set_preview_widget(preview)
        filew.connect("update-preview", update_image)

        response = filew.run()
        if response == Gtk.ResponseType.OK:
            global iconpath
            iconpath = filew.get_filename()
            new_icon = Pixbuf.new_from_file_at_size(iconpath, 32, 32)
            icon.set_from_pixbuf(new_icon)
            filew.destroy()
        elif response == Gtk.ResponseType.CANCEL:
            filew.destroy()

class NoBrowserError(Gtk.Window):
    
    def destroy(self, button):
        self.close()
        
    def __init__(self):
        Gtk.Window.__init__(self, title="Browser Error")
        self.set_size_request(250, 130)
        self.set_icon_from_file("/usr/share/pixmaps/ice.png")
        
        print("test")

        main_lab = Gtk.Label()
        main_lab.set_markup("<b>Warning: No Suitable Browser Detected</b>")
        text_lab = Gtk.Label("The name of the SSB will cause an existing SSB to\nbe overwritten. To prevent this, change a letter in\nthe name. Continue anyway?")
        text_lab = Gtk.Label("Ice requires a system installation of either Google\nChrome or Chromium in order to function. Please\ninstall at least one in order to create SSBs.")

        close = Gtk.Button.new_from_stock(Gtk.STOCK_CLOSE)
        close.connect("clicked", self.destroy)
        void = Gtk.Label()
        box = Gtk.HBox()
        box.pack_start(void, True, True, 0)
        box.pack_start(close, False, False, 0)
        
        main_vbox = Gtk.VBox()
        main_vbox.pack_start(main_lab, False, False, 10)
        main_vbox.pack_start(text_lab, False, False, 0)
        main_vbox.pack_start(box, False, False, 10)
        
        main_hbox = Gtk.HBox()
        main_hbox.pack_start(main_vbox, True, True, 10)
        self.add(main_hbox)
        self.show_all()

class DuplicateError(Gtk.Window):
    
    def destroy(self, button):
        self.close()
        
    def okay_clicked(self, button, title, formatted, address, iconext, location):
        
        for item in liststore:
            itemiter = item.iter
            semiformatted = ""
            array = filter(str.isalpha, item[1])
            for obj in array:
                semiformatted = semiformatted + obj
            forma = semiformatted.lower()
            
            if forma == formatted:
                liststore.remove(itemiter)

        writefile(title, formatted, address, iconext, location)
        self.close()

    def __init__(self, title, formatted, address, iconext, location):
        Gtk.Window.__init__(self, title="Duplication Error")
        self.set_size_request(250, 130)
        self.set_icon_from_file("/usr/share/pixmaps/ice.png")
        
        main_lab = Gtk.Label()
        main_lab.set_markup("<b>Warning: File Duplication Error</b>")
        text_lab = Gtk.Label("The name of the SSB will cause an existing SSB to\nbe overwritten. To prevent this, change a letter in\nthe name. Continue anyway?")

        okay = Gtk.Button.new_from_stock(Gtk.STOCK_OK)
        okay.connect("clicked", self.okay_clicked, title, formatted, address, iconext, location)
        cancel = Gtk.Button.new_from_stock(Gtk.STOCK_CANCEL)
        cancel.connect("clicked", self.destroy)
        void = Gtk.Label()
        box = Gtk.HBox()
        box.pack_start(void, True, True, 0)
        box.pack_start(okay, False, False, 10)
        box.pack_start(cancel, False, False, 0)
        
        main_vbox = Gtk.VBox()
        main_vbox.pack_start(main_lab, False, False, 10)
        main_vbox.pack_start(text_lab, False, False, 0)
        main_vbox.pack_start(box, False, False, 10)
        
        main_hbox = Gtk.HBox()
        main_hbox.pack_start(main_vbox, True, True, 10)
        self.add(main_hbox)
        self.show_all()

class AddressError(Gtk.Window):
    
    def destroy(self, button):
        self.close()
        
    def okay_clicked(self, button):
        applicate()
        self.close()
               
    def __init__(self):
        Gtk.Window.__init__(self, title="Address Error")
        self.set_size_request(250, 130)
        self.set_icon_from_file("/usr/share/pixmaps/ice.png")
        
        main_lab = Gtk.Label()
        main_lab.set_markup("<b>Warning: HTTP or URL Error</b>")
        text_lab = Gtk.Label("An error with the web address has been detected.\nThis is possibly the site being down or unavailable\nright now. Continue anyway?")

        okay = Gtk.Button.new_from_stock(Gtk.STOCK_OK)
        okay.connect("clicked", self.okay_clicked)
        cancel = Gtk.Button.new_from_stock(Gtk.STOCK_CANCEL)
        cancel.connect("clicked", self.destroy)
        void = Gtk.Label()
        box = Gtk.HBox()
        box.pack_start(void, True, True, 0)
        box.pack_start(okay, False, False, 10)
        box.pack_start(cancel, False, False, 0)
        
        main_vbox = Gtk.VBox()
        main_vbox.pack_start(main_lab, False, False, 10)
        main_vbox.pack_start(text_lab, False, False, 0)
        main_vbox.pack_start(box, False, False, 10)
        
        main_hbox = Gtk.HBox()
        main_hbox.pack_start(main_vbox, True, True, 10)
        self.add(main_hbox)
        self.show_all()

class Ice(Gtk.Window):
    
    def destroy(self, button):
        Gtk.main_quit()
        
    def icon_select(self, button):
        IconSel()

    def apply_clicked(self, button):
        if errortest(normalize(url.get_text())) == True:
            applicate()
        elif errortest(normalize(url.get_text())) == False:
            AddressError()
        elif errortest(normalize(url.get_text())) == None:
            print("ERROR: An address error has occurred.")
            sys.exit(1)
        else:
            print("ERROR: An unknown error has occurred.")
            sys.exit(1)

    def icon_download(self, button):
        appurl = normalize(url.get_text())
        global iconpath
        try:
            download_favicon(appurl)
            addr0 = get_favicon_url(appurl)
            addr1 = addr0.replace('/', ' ')
            addr2 = addr1.split()[-1]
            iconpath = "/tmp/" + addr2
            new_icon = Pixbuf.new_from_file_at_size(iconpath, 32, 32)
            icon.set_from_pixbuf(new_icon)
        except:
            iconpath = "/usr/share/pixmaps/ice.png"
            new_icon = Pixbuf.new_from_file_at_size(iconpath, 32, 32)
            icon.set_from_pixbuf(new_icon)
           
    def __init__(self):
        Gtk.Window.__init__(self, title="Ice")
        self.current_directory = os.path.realpath(os.path.expanduser('~') + "/.local/share/applications/")
        self.set_size_request(460, 350)
        self.set_icon_from_file("/usr/share/pixmaps/ice.png")
        
        ######################
        ### 'Create' page. ###
        ######################
        
        welcome = Gtk.Label()
        welcome.set_markup("<b>Welcome to Ice, a simple SSB manager.</b>")
        global name
        name = Gtk.Entry()
        name.set_placeholder_text("Name the application")
        global url
        url = Gtk.Entry()
        url.set_placeholder_text("Enter web address")
        
        where_store = ["Accessories", "Games", "Graphics", "Internet", "Office", "Programming", "Sound & Video", "System Tools"]
        where_lab = Gtk.Label("Where in the menu?")
        global where
        where = Gtk.ComboBoxText()
        where.set_entry_text_column(0)
        for entry in where_store:
            where.append_text(entry)
        where.set_active(3)
        
        where_box = Gtk.HBox()
        where_void = Gtk.Label()
        where_box.pack_start(where_lab, False, False, 0)
        where_box.pack_start(where_void, False, False, 10)
        where_box.pack_start(where, True, True, 0)
        
        global iconpath
        iconpath = "/usr/share/pixmaps/ice.png"
        icon_pixbuf = Pixbuf.new_from_file_at_size(iconpath, 32, 32)
        global icon
        icon = Gtk.Image()
        icon.set_from_pixbuf(icon_pixbuf)
        
        icon_void = Gtk.Label()
        icon_box = Gtk.HBox()
        icon_box.pack_start(icon, False, False, 10)
        icon_box.pack_start(icon_void, False, False, 10)
        
        choose_icon = Gtk.Button("Select an icon")
        choose_icon.connect("clicked", self.icon_select)
        download_icon = Gtk.Button("Use site favicon")
        download_icon.connect("clicked", self.icon_download)
        
        icon_vbox = Gtk.VBox()
        icon_vbox.pack_start(choose_icon, True, True, 5)
        icon_vbox.pack_start(download_icon, True, True, 5)
        icon_hbox = Gtk.HBox()
        icon_hbox.pack_start(icon_box, False, False, 10)
        icon_hbox.pack_start(icon_vbox, True, True, 0)
        
        global firefox
        firefox = Gtk.RadioButton.new_with_label_from_widget(None, "Firefox")
        if not os.path.exists("/usr/bin/firefox"):
            firefox.set_sensitive(False)
        if not os.path.exists("/usr/bin/chromium-browser") and not os.path.exists("/usr/bin/google-chrome") and os.path.exists("/usr/bin/firefox"):
            chrome.set_active(True)

        global chrome
        chrome = Gtk.RadioButton.new_from_widget(firefox)
        chrome.set_label("Chrome")
        if not os.path.exists("/usr/bin/google-chrome"):
            chrome.set_sensitive(False)
        if not os.path.exists("/usr/bin/chromium-browser") and not os.path.exists("/usr/bin/firefox") and os.path.exists("/usr/bin/google-chrome"):
            chrome.set_active(True)

        global chromium
        chromium = Gtk.RadioButton.new_from_widget(chrome)
        chromium.set_label("Chromium")
        if not os.path.exists("/usr/bin/chromium-browser"):
            chromium.set_sensitive(False)
        if not os.path.exists("/usr/bin/google-chrome") and not os.path.exists("/usr/bin/firefox") and os.path.exists("/usr/bin/chromium-browser"):
            chromium.set_active(True)
        
        apply_button = Gtk.Button.new_from_stock(Gtk.STOCK_APPLY)
        apply_button.connect("clicked", self.apply_clicked)
        close_button = Gtk.Button.new_from_stock(Gtk.STOCK_CLOSE)
        close_button.connect("clicked", self.destroy)
        button_void = Gtk.Label()
        button_box = Gtk.HBox()
        button_box.pack_start(chrome, False, False, 0)
        button_box.pack_start(chromium, False, False, 10)
        button_box.pack_start(firefox, False, False, 0)
        button_box.pack_start(button_void, True, True, 0)
        button_box.pack_start(apply_button, False, False, 20)
        button_box.pack_start(close_button, False, False, 0)
        
        create_vbox = Gtk.VBox()
        create_vbox.pack_start(welcome, False, False, 15)
        create_vbox.pack_start(name, False, False, 0)
        create_vbox.pack_start(url, False, False, 10)
        create_vbox.pack_start(where_box, False, False, 10)
        create_vbox.pack_start(icon_hbox, False, False, 10)
        create_vbox.pack_start(button_box, False, False, 0)
        
        create_hbox = Gtk.HBox()
        create_hbox.pack_start(create_vbox, True, True, 20)
        create_lab = Gtk.Label("Create")
        
        ######################
        ### 'Remove' page. ###
        ######################
        
        global liststore
        liststore = Gtk.ListStore(Pixbuf, str)
        for fl in os.listdir(os.path.expanduser("~/.local/share/applications")):
            a = os.path.expanduser("~/.local/share/applications") + '/' + fl
            if not os.path.isdir(a):
                nameline, pixbuf = get_details(a)
                if not nameline == None and not pixbuf == None:
                    liststore.append([pixbuf, nameline])

        global iconview
        iconview = Gtk.IconView()
        iconview.set_model(liststore)
        iconview.set_pixbuf_column(0)
        iconview.set_text_column(1)
        iconview.set_selection_mode(1)
        
        scroll = Gtk.ScrolledWindow()
        scroll.add(iconview)
        
        remove = Gtk.Button.new_from_stock(Gtk.STOCK_REMOVE)
        remove.connect("clicked", delete)
        close = Gtk.Button.new_from_stock(Gtk.STOCK_CLOSE)
        close.connect("clicked", self.destroy)
        void = Gtk.Label()
        buttons = Gtk.HBox()
        buttons.pack_start(void, True, True, 0)
        buttons.pack_start(remove, False, False, 20)
        buttons.pack_start(close, False, False, 0)
        
        remove_vbox = Gtk.VBox()
        remove_vbox.pack_start(scroll, True, True, 10)
        remove_vbox.pack_start(buttons, False, False, 17)
        
        remove_hbox = Gtk.HBox()
        remove_hbox.pack_start(remove_vbox, True, True, 20)
        remove_lab = Gtk.Label("Remove")
        
        ##########################
        ### Main window stuff. ###
        ##########################

        notebook = Gtk.Notebook()
        notebook.append_page(create_hbox, create_lab)
        notebook.append_page(remove_hbox, remove_lab)
        
        main_vbox = Gtk.VBox()
        main_vbox.pack_start(notebook, True, True, 10)
        main_hbox = Gtk.HBox()
        main_hbox.pack_start(main_vbox, True, True, 10)
        self.add(main_hbox)
        self.show_all()

        if not os.path.exists("/usr/bin/google-chrome") and not os.path.exists("/usr/bin/chromium-browser") and not os.path.exists("/usr/bin/firefox"):
            apply_button.set_sensitive(False)
            NoBrowserError()

window = Ice()
window.connect("delete-event", Gtk.main_quit)
Gtk.main()

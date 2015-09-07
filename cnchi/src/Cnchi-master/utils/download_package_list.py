import urllib.request
import urllib.error

try:
    packages_xml = urllib.request.urlopen('http://install.antergos.com/packages-0.4.xml')
except urllib.error.URLError as e:
    # If the installer can't retrieve the remote file, try to install with a local
    # copy, that may not be updated
    print(e)


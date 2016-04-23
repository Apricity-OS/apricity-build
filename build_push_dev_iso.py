from subprocess import call
from os import chdir, path, getcwd, listdir

class cd:
    """Context manager for changing the current working directory"""
    def __init__(self, newPath):
        self.newPath = path.expanduser(newPath)

    def __enter__(self):
        self.savedPath = getcwd()
        chdir(self.newPath)

    def __exit__(self, etype, value, traceback):
        chdir(self.savedPath)

with cd('~/Apricity-OS/apricity-build'):
    for edition in ['gnome', 'cinnamon']:
        for arch in ['x84_64']:
            call('su -c "./build.sh -v -o dev -E ' + edition + '-A ' + arch + '"', shell=True)

    call(['rsync', '-aP', 'dev/', 'apricity@apricityos.com:public_html/iso-dev'])

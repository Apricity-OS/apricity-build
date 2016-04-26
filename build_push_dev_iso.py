from subprocess import call, check_call
from os import chdir, path, getcwd, listdir, walk
from shutil import rmtree

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
    try:
        check_call('sudo rm -r dev', shell=True)
    except Exception as e:
        print(e)
    for edition in ['gnome', 'cinnamon']:
        for arch in ['x84_64']:
            call('su -c "./build.sh -v -o dev -E ' + edition + ' -A ' + arch + '"', shell=True)

    dirpath, dirnames, filenames = walk('dev')
    if len(filenames) > 0:
        call(['rsync', '-aP', 'dev/', 'apricity@apricityos.com:public_html/iso-dev'])
    else:
        print('Failed build')

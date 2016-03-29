from subprocess import call

for edition in ['gnome', 'cinnamon']:
    for arch in ['x84_64']:
        call(['su -c "./build.sh -v -E ' + edition + '-A' + arch + '"'], shell=True)

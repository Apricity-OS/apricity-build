from subprocess import call

for edition in ['gnome', 'cinnamon']:
    for arch in ['x84_64']:
        call(['./build.sh', '-E', edition, '-A', arch])

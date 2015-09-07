#!/usr/bin/python

import xml.etree.ElementTree as etree
from urllib.request import urlopen

packages = []

packages_xml = urlopen('http://install.cinnarch.com/packages.xml')
tree = etree.parse(packages_xml)
root = tree.getroot()
for child in root.iter('base_system'):
    for pkg in child.iter('pkgname'):
        packages.append(pkg.text)

for n in packages:
    print(n)

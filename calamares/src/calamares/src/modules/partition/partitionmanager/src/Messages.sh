#! /usr/bin/env bash
$EXTRACTRC `find -name \*.rc` >> rc.cpp || exit 11
$EXTRACTRC `find -name \*.ui` >> rc.cpp || exit 12
$EXTRACTRC `find -name \*.kcfg` >> rc.cpp || exit 12
$XGETTEXT `find -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/partitionmanager.pot
rm -f rc.cpp

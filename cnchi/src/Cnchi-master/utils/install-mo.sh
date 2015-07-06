#!/bin/sh
for filename in /usr/share/cnchi/po/*.po; do
    bname=$(basename "$filename" .po)
    messages_dir="/usr/share/locale/$bname/LC_MESSAGES"
    #mkdir -p $messages_dir
    msgfmt -o $messages_dir/cnchi.mo $filename
done

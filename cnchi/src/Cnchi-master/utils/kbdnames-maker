#!/usr/bin/perl

use warnings 'all';
use strict;
use Locale::gettext;
use POSIX;

BEGIN {
    my $file;
    if ($ARGV[0]) {
	$file = $ARGV[0];
    } else {
	$file = 'KeyboardNames.pl';
    }
    do "$file";
}

for my $model (keys %KeyboardNames::models) {
    my $name = $KeyboardNames::models{$model};
    print "C*model*$name*$model\n"
}
for my $layout (keys %KeyboardNames::layouts) {
    my $name = $KeyboardNames::layouts{$layout};
    print "C*layout*$name*$layout\n";
    print "C*variant*$name**$layout\n";
    for my $variant (keys %{$KeyboardNames::variants{$name}}) {
	my $variantname = $KeyboardNames::variants{$name}{$variant};
	print "C*variant*$name*$variantname*$layout - $variant\n";
    }
}

# Make sure we output UTF-8
$ENV{'LC_ALL'} = "en_US.UTF-8";
binmode STDOUT, ":utf8";

for my $mo (</usr/share/locale/*/LC_MESSAGES/xkeyboard-config.mo>) {
    my $lang = $mo;
    $lang =~ s:/usr/share/locale/(.*)/LC_MESSAGES/xkeyboard-config.mo:$1:;
    $ENV{'LANGUAGE'} = $lang;
    setlocale(LC_ALL,"");

    $lang =~ s:\@:__:;
    $lang =~ s:__Latn:__latin:; # special fixup for sr

    my $d = Locale::gettext->domain("xkeyboard-config");

    for my $model (keys %KeyboardNames::models) {
	my $name = $KeyboardNames::models{$model};
	print "$lang*model*$name*".($d->get($model))."\n"
    }
    for my $layout (keys %KeyboardNames::layouts) {
	my $name = $KeyboardNames::layouts{$layout};
	my $local_layout = $d->get($layout);
	print "$lang*layout*$name*$local_layout\n";
	print "$lang*variant*$name**$local_layout\n";
	for my $variant (keys %{$KeyboardNames::variants{$name}}) {
	    my $variantname = $KeyboardNames::variants{$name}{$variant};
	    print "$lang*variant*$name*$variantname*$local_layout - ".($d->get($variant))."\n";
	}
    }
}

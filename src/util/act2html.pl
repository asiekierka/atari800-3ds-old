#!/usr/bin/perl
use strict;

if (@ARGV == 0) {
	print "Usage: perl act2html.pl INPUT.act... > OUTPUT.html\n";
	exit;
}
my @files = glob "@ARGV";
@files > 0 or die "act2html.pl: no input file\n";

my @palettes;
for (@files) {
	my $pal;
	open IN, $_ and read IN, $pal, 769 and close IN or die "act2html.pl: $_: $!\n";
	die "act2html.pl: $_ is too short\n" if length($pal) < 768;
	die "act2html.pl: $_ is too long\n" if length($pal) > 768;
	push @palettes, $pal;
}

print qq{<html>\n<head>\n<title>Generated by act2html.pl</title>\n</head>\n};
print qq{<body>\n<table width="100%">\n<tr><th>file</th>};
print qq{<th width="5%">$_</th>} for 0 .. 15;
print qq{</tr>\n};
for my $hue (0 .. 15) {
	print qq{<tr><th colspan="17">Hue: $hue</th></tr>\n};
	for my $i (0 .. $#files) {
		print qq{<tr><td>$files[$i]</td>};
		for (my $color = $hue * 16; ; ) {
			my $rgb = substr($palettes[$i], $color * 3, 3);
			my $colspan = 1;
			while (($color & 0xf) < 0xf && $rgb eq substr($palettes[$i], $color * 3 + 3, 3)) {
				$colspan++;
				$color++;
			}
			print qq{<td bgcolor="#}, unpack('H6', $rgb);
			print qq{" colspan="$colspan} if $colspan > 1;
			print qq{"></td>};
			++$color & 0xf or last;
		}
		print qq{</tr>\n};
	}
}
print qq{</table>\n</body>\n</html>\n};
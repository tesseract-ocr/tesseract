#!/usr/bin/perl

use warnings;
use strict;
use utf8;

use Getopt::Std;

=pod

=head1 NAME 

genwordlists.pl - generate word lists for Tesseract

=head1 SYNOPSIS

genwordlists.pl -i large_text_file -d outdir -p lang

=head1 DESCRIPTION

    genwordlists.pl -i large_text_file -d outdir -p lang

Creates 4 files in C<outdir>: F<lang.word.bigrams.unsorted>,
F<lang.word.numbers.unsorted>, F<lang.word.punc.unsorted>, and
F<lang.wordlist.unsorted>, which (when sorted) can be used with
C<wordlist2dawg> for Tesseract's language data.

The script can also run as a filter. Given a set of files created
by WikiExtractor (L<http://medialab.di.unipi.it/Project/SemaWiki/Tools/WikiExtractor.py>),
use:

    find WikiExtractor -type f | while read i; do \
    pfx=$(echo $i|tr '/' '_'); cat $i | \
    perl genwordlists.pl -d OUTDIR -p $pfx; done

This will create a set of output files to match each of the files 
WikiExtractor created.

To combine these files:

    for i in word.bigrams.unsorted word.numbers.unsorted \
    word.punc.unsorted wordlist.unsorted; do \
    find OUTDIR -name "*$i" -exec cat '{}' \; |\
    perl -CS -ane 'BEGIN{my %c=();} chomp;
    my($a,$b)=split/\t/;if(defined $c{$a}){$c{$a}+=$b}
    else {$c{$a} = $b;} END{while(my($k,$v)=each %c)
    {print "$v\t$k\n";}}'|sort -nr > tmp.$i ;done

Followed by:

    for i in word.punc.unsorted word.bigrams.unsorted \
    word.numbers.unsorted;do cat tmp.$i \
    awk -F'\t' '{print $2 "\t" $1}' > real.$i ; done
    cat tmp.wordlist.unsorted | awk -F'\t' '{print $2}' \
    > real.wordlist.unsorted

Note that, although the langdata repository contains the
counts of each item in most of the punctuation, number, and
bigram files, these files must be filtered to only contain
the first column, otherwise C<wordlist2dawg> will fail to write
the output file.

=head1 CAVEATS

The format of the output files, and how the data are extracted,
is based only on staring at the input files and taking a guess.
They may be wildly inaccurate.

The only part I can say for certain is correct is that digits
are replaced with '?' in the .numbers wordlist. (See F<dict/dict.cpp>
in the Tesseract source).

=head1 COPYRIGHT

Copyright 2014 Jim O'Regan

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

L<http://www.apache.org/licenses/LICENSE-2.0>

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=head1 SEE ALSO

L<wordlist2dawg(1)>

=cut

# I haven't looked into this too much
my %lig = (
	# Longest first
	'ffi' => 'ﬃ',
	'ct' => "\N{U+E003}",
	'ff' => 'ﬀ',
	'fi' => 'ﬁ',
	'fl' => 'ﬂ',
	'st' => 'ﬆ',
);

my %punct;
my %num;
my %bigrams;
my %opts;
my %words;

my $do_ligatures = 0;

getopts("hli:p:d:", \%opts);

if (defined $opts{h}) {
	print "Usage: genwordlists [options]\n";
	print "-h\tPrints a brief help message\n";
	print "-d\tSet the output directory (default is current)\n";
	print "-b\tSet the prefix for the language data (e.g., eng for English)\n";
	print "-l\tProcess ligatures\n";
	print "-i\tSet the input file. If not set, reads from stdin\n";
	exit;
}

if (defined $opts{l}) {
	$do_ligatures = 1;
}

my $prefix = '';
if (!defined $opts{p}) {
	print "Prefix (-p) must be set!\n";
	exit;
} else {
	if (defined $opts{d}) {
		$prefix = $opts{d};
		$prefix =~ s/\/$//;
		$prefix .= '/';
	}
	$prefix .= $opts{p};
	# Easiest is to drop it, if present, and readd
	$prefix =~ s/\.$//;
	$prefix .= ".";
}

my $input;
if (defined $opts{i}) {
	open ($input, "<", $opts{i}) or die $!;
#} elsif ($#ARGV > 0) {
#	open ($input, "<", $ARGV[0]) or die $!;
} else {
	$input = *STDIN;
}
binmode $input, ":utf8";

while (<$input>) {
	chomp;
	tr/\t/ /;

	next if (/^<doc/);
	next if (/^<\/doc/);
	next if (/^$/);
	next if (/^[ \t]*$/);
	next if (/^\]\]$/);

	my @punct = $_ =~ /([ \p{Punct}]*)/g;
	for my $i (@punct) {
		if(defined($punct{$i})) {
			$punct{$i}++;
		} else {
			$punct{$i} = 1;
		}
	}
	my @rawnumtok = split(/ /);
	my @numtok = map { local $_ = $_; s/[0-9]/ /g; $_ } grep(/[0-9]/, @rawnumtok);
	for my $i (@numtok) {
		if(defined($num{$i})) {
			$num{$i}++;
		} else {
			$num{$i} = 1;
		}
	}

	my @bitoksraw = map { local $_ = $_; s/[0-9]/?/g; $_ } split(/ |[ \p{Punct}][ \p{Punct}]+/);
	if ($#bitoksraw > 0) {
		my @first = @bitoksraw;
		my $discard = shift @bitoksraw;
		for (my $j = 0; $j != $#first; $j++) {
			if ($bitoksraw[$j] ne '' && $first[$j] ne '') {
				my $tok = $first[$j] . " " . $bitoksraw[$j];
				#Not keeping count of these, but this can be useful for trimming
				if(defined($bigrams{$tok})) {
					$bigrams{$tok}++;
				} else {
					$bigrams{$tok} = 1;
				}
				if($do_ligatures == 1) {
					my $other = do_lig($tok);
					if ($other ne $tok) {
						if(defined($bigrams{$other})) {
							$bigrams{$other}++;
						} else {
							$bigrams{$other} = 1;
						}
					}
				}
			}
		}
	}
	my @wordl = grep { !/[0-9 \p{Punct}]/ } split (/[ \p{Punct}]+/);
	if ($#wordl >= 0) {
		for my $word (@wordl) {
			if (defined $words{$word}) {
				$words{$word}++;
			} else {
				$words{$word} = 1;
			}
		}
	}
}

if (defined $opts{i}) {
	close $input;
}

open(BIGRAMS, ">", "${prefix}word.bigrams.unsorted");
binmode BIGRAMS, ":utf8";
while (my($k, $v) = each %bigrams) {
	print BIGRAMS "$k\t$v\n";
}
close BIGRAMS;
%bigrams = ();

open(PUNCT, ">", "${prefix}word.punc.unsorted");
binmode PUNCT, ":utf8";
while (my($k, $v) = each %punct) {
	print PUNCT "$k\t$v\n";
}
close PUNCT;
%punct = ();

open(NUMS, ">", "${prefix}word.numbers.unsorted");
binmode NUMS, ":utf8";
while (my($k, $v) = each %num) {
	print NUMS "$k\t$v\n";
}
close NUMS;
%num = ();

open(WORDS, ">", "${prefix}wordlist.unsorted");
binmode WORDS, ":utf8";
while (my($k, $v) = each %words) {
	print WORDS "$k\t$v\n";
}
close WORDS;
%words = ();

sub do_lig {
	my $word = shift;
	while (my($k, $v) = each %lig) {
		$word =~ s/$k/$v/g;
	}
	$word;
}

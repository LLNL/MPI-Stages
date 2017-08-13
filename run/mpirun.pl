#!/usr/bin/perl
# mpirun.pl
# Initiate MPI launch to hosts specified in MPIHOSTS
# Send data format <$bin;$nextrank;$sz;@hosts#> where '#' indicates end of data and each data is separated by ';'.

use v5.10;
use strict;
use warnings;
use IO::Socket::INET;

my ($bin) = @ARGV;
my $MPISOCKET = 4422;

open(my $hostsfh, "<", "MPIHOSTS") or die "Can't read MPIHOSTS";
my @hosts = <$hostsfh>;
chomp @hosts;
my $sz = scalar @hosts;
my $nextrank = 0;
my $host;

my %config;
$config{"size"} = "$sz";
say "Generating config table...";
foreach $host (@hosts)
{
  say "Assigning $nextrank to $host";
  $config{"$nextrank"} = "$host";
  $nextrank += 1;
}



say "Generating config list and string...";
my @configlist = ();
foreach my $k (keys %config)
{
  say "Registering $k:$config{$k}";
  push (@configlist, "$k:$config{$k}");
}
my $configstr = join(";",@configlist);


$nextrank = 0;
say "Sending config str and rank assignments...";
foreach $host (@hosts)
{
  say "Assigning rank $nextrank to $host...";
  my $s = new IO::Socket::INET(
    PeerHost => $host,
    PeerPort => $MPISOCKET,
    Proto => 'tcp',
  );

  die "Failed connecting to $host:$MPISOCKET..." unless $s;
  say $s $bin;
  say $s $nextrank;
  say $s $configstr;
  $nextrank += 1;
  $s->close();
}

say "Done!";

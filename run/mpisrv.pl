#!/usr/bin/perl
# mpisrv.pl
# listening server for mpirun

use v5.10;
use strict;
use warnings;
use IO::Socket::INET;
$| = 1;
 
my $MPISOCKET = 4422;

my $listen = new IO::Socket::INET (
  LocalHost => '0.0.0.0',
  LocalPort => $MPISOCKET,
  Proto => 'tcp',
  Listen => 5,
  Reuse => 1
);
die "Couldn't create listening socket; $!" unless $listen;

while(1)
{
  say "Waiting for mpirun...";
  my $s = $listen->accept();
  my $bin = <$s>;
  chomp $bin;
  my $rank = <$s>;
  chomp $rank;
  my $epoch = <$s>;
  chomp $epoch;
  my $configstr = <$s>;
  chomp $configstr;
  my $filename = "mpirun.$rank.tmp";

  say "Received:";
  say "  bin = $bin";
  say "  rank = $rank";
  say "  epoch = $epoch";
  say "  configstr = $configstr";


  open(my $config, ">", $filename) or die "Couldn't create temporary file";
  print $config join("\n", split(/;/, $configstr));

  my @args = ("$filename", "$rank", "$epoch", "$bin");
  push(@args, @ARGV);


  say "Launching $bin as rank $rank";
  say $s system $bin @args;

  shutdown($s, 1);
}

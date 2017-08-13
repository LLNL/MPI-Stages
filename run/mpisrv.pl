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
  my $s = $listen->accept();
  my $bin = <$s>;
  chomp $bin;
  my $rank = <$s>;
  chomp $rank;
  my $configstr = <$s>;
  chomp $configstr;
  my $filename = "mpirun.$rank.tmp";

  say "Received:";
  say "  bin = $bin";
  say "  rank = $rank";
  say "  configstr = $configstr";


  open(my $config, ">", $filename) or die "Couldn't create temporary file";
  print $config join("\n", split(/;/, $configstr));

  my @args = ("$filename", "$rank", "$bin");
  push(@args, @ARGV);


  say "Launching $bin as rank $rank";
  system $bin @args;
  shutdown($s, 1);
}

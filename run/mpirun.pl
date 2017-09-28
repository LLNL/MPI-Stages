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

say "Starting at epoch 0...";
my $epoch = 0;

my $done = 0;
until($done)
{
  say "Launching processes at epoch $epoch";

  $nextrank = 0;
  my @s;
  say "Sending config str and rank assignments...";
  foreach $host (@hosts)
  {
    say "Assigning rank $nextrank to $host...";
    $s[$nextrank] = new IO::Socket::INET(
      PeerHost => $host,
      PeerPort => $MPISOCKET,
      Proto => 'tcp',
    );

    die "Failed connecting to $host:$MPISOCKET..." unless $s;
    say $s[$nextrank] $bin;
    say $s[$nextrank] $nextrank;
    say $s[$nextrank] $epoch;
    say $s[$nextrank] $configstr;
    $nextrank += 1;
  }

  say "Gathering results...";
  my $lowest = 999999;
  $done = 1;
  $nextrank = 0;
  foreach $host(@hosts)
  {
    my $result = <$s>;
    chomp $result;
    if($result > 0 and $result < $lowest)
    {
      say "Got nonzero return $result from $host";
      $done = 0;
      $lowest = $result;
    }
  }

}
say "Done!";

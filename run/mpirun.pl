#!/usr/bin/perl
# mpirun.pl
# Initiate MPI launch to hosts specified in MPIHOSTS
# Send data format <$bin;$nextrank;$sz;@hosts#> where '#' indicates end of data and each data is separated by ';'.

use v5.10;
use strict;
use warnings;
use IO::Select;
use IO::Socket::INET;

sub SendCmd
{
  my $hr = shift;
  my $haddr = shift;
  my $str = shift;
  say "SendCmd to $haddr:  $str";
  say {$$hr{$haddr}{sock}} $str;
}

sub SendAllCmd
{
  my $hr = shift;
  my $str = shift;
  say "SendAllCmd:  $str";
  foreach my $h (keys %$hr)
  {
    SendCmd($hr, $h, $str);
  }
}

my ($bin) = @ARGV;
my $MPISOCKET = 4422;

my $sel = IO::Select->new();

say "Loading MPIHOSTS...";
open(my $hostsfh, "<", "MPIHOSTS") or die "Can't read MPIHOSTS";
my @hosts = <$hostsfh>;
chomp @hosts;
my $sz = scalar @hosts;
my $nextrank = 0;
my $host;
my %nodes;
my $rnodes = \%nodes;

my %config;
$config{"size"} = "$sz";
say "Generating node table...";
foreach $host (@hosts)
{
  say "\tAssigning $nextrank to $host";
  $nodes{$host}{rank} = $nextrank;
  $nextrank += 1;
}

say "Generating config string...";
my @configlist = ("size:$sz");
foreach my $k (keys %nodes)
{
  say "Registering $nodes{$k}{rank}:$k";
  push (@configlist, "$nodes{$k}{rank}:$k");
}
my $configstr = join(";",@configlist);

say "Starting at epoch 0...";
my $epoch = 0;

say "Opening connection to all ranks...";
foreach my $h (keys %nodes)
{
  $nodes{$h}{sock} = new IO::Socket::INET(
      PeerHost => $h,
      PeerPort => $MPISOCKET,
      Proto => 'tcp',
    );
  die "\t$h\tFAILED" unless $nodes{$h}{sock};
  say "\t$h\tOK";
  $nodes{$h}{sock}->autoflush(1);
  $sel->add($nodes{$h}{sock});
}

say "Sending basic init...";
SendAllCmd($rnodes, "bin\n$bin\nepoch\n$epoch\nconfigstr\n$configstr");
foreach my $h (keys %nodes)
{
  #say { $nodes{$h}{sock} } "bin\n$bin";
  #say { $nodes{$h}{sock} } "rank\n$nodes{$h}{rank}";
  #say { $nodes{$h}{sock} } "epoch\n$epoch";
  #say { $nodes{$h}{sock} } "configstr\n$configstr";
  SendCmd($rnodes, $h, "rank\n$nodes{$h}{rank}");
}

say "Launching...";
SendAllCmd($rnodes, "!run");
#foreach my $h (keys %nodes)
#{
#  SendCmd()
#  say { $nodes{$h}{sock} } "!run";
#}


my $live = scalar keys %nodes;
my $done = 0;
my $latest = 1e9;
say "Starting wait on $live nodes...";
until($done)
{
  $done = 1;
  while($live)
  {
    my @ready = $sel->can_read;
    foreach my $s (@ready)
    {
      $live--;
      say "Got response; now $live live";
      my $return = <$s>;
      chomp $return;
      my $lastepoch = <$s>;
      chomp $lastepoch;
      if($lastepoch < $latest)
      {
        say "Updating to epoch $lastepoch";
        $latest = $lastepoch;
      }
      if($return == 0)
      {
        say "Got OK return";
      }
      else
      {
        $done = 0;
        my $sig = $return & 127;
        my $st = $return >> 8;
        say "Bad return!";
        say "\t Signal $sig";
        say "\t Status $st";
        say "Sending kills...";
        SendAllCmd($rnodes, "!kill");
        #foreach my $h (keys %nodes)
        #{
        #  say { $nodes{$h}{sock} } "!kill";
        #}
      }
    }
  }
  unless($done)
  {
    $live = scalar keys %nodes;
    $epoch = $latest;
    $latest = 1e9;
    say "Not done, so relaunching all in epoch $epoch";
    SendAllCmd($rnodes, "epoch\n$epoch\n!run");
    #foreach my $h (keys %nodes)
    #{
    #  say { $nodes{$h}{sock} } "epoch\n$epoch\n!run";
    #
  } 
}

SendAllCmd($rnodes, "!done");
say "Finished!  Exiting...";

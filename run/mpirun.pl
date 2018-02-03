#!/usr/bin/perl
# mpirun.pl
# Initiate MPI launch to hosts specified in MPIHOSTS
# Send data format <$bin;$nextrank;$sz;@hosts#> where '#' indicates end of data and each data is separated by ';'.

use v5.10;
use strict;
use warnings;
use IO::Select;
use IO::Socket::INET;
$| = 1;

sub SendCmd
{
  my $hr = shift;
  my $haddr = shift;
  my $str = shift;
  print "SendCmd to $haddr:  $str\n";
  print {$$hr{$haddr}{sock}} "$str\n";
}

sub SendAllCmd
{
  my $hr = shift;
  my $str = shift;
  print "SendAllCmd:  $str\n";
  foreach my $h (keys %$hr)
  {
    SendCmd($hr, $h, $str);
  }
}

my ($bin) = @ARGV;
my $MPISOCKET = 4422;

my $sel = IO::Select->new();

print "Loading MPIHOSTS...\n";
open(my $hostsfh, "<", "MPIHOSTS") or die "Can't read MPIHOSTS";
$hostsfh->autoflush(1);
my @hosts = <$hostsfh>;
chomp @hosts;
my $sz = scalar @hosts;
my $nextrank = 0;
my $host;
my %nodes;
my $rnodes = \%nodes;

my %config;
$config{"size"} = "$sz";
print "Generating node table...\n";
foreach $host (@hosts)
{
  print "\tAssigning $nextrank to $host\n";
  $nodes{$host}{rank} = $nextrank;
  $nextrank += 1;
}

print "Generating config string...\n";
my @configlist = ("size:$sz");
foreach my $k (keys %nodes)
{
  print "Registering $nodes{$k}{rank}:$k\n";
  push (@configlist, "$nodes{$k}{rank}:$k");
}
my $configstr = join(";",@configlist);

print "Starting at epoch 0...\n";
my $epoch = 0;

print "Opening connection to all ranks...\n";
foreach my $h (keys %nodes)
{
  $nodes{$h}{sock} = new IO::Socket::INET(
      PeerHost => $h,
      PeerPort => $MPISOCKET,
      Proto => 'tcp',
    );
  die "\t$h\tFAILED" unless $nodes{$h}{sock};
  print "\t$h\tOK\n";
  $nodes{$h}{sock}->autoflush(1);
  $sel->add($nodes{$h}{sock});
}

print "Sending basic init...\n";
SendAllCmd($rnodes, "bin\n$bin\nepoch\n$epoch\nconfigstr\n$configstr");
foreach my $h (keys %nodes)
{
  #say { $nodes{$h}{sock} } "bin\n$bin";
  #say { $nodes{$h}{sock} } "rank\n$nodes{$h}{rank}";
  #say { $nodes{$h}{sock} } "epoch\n$epoch";
  #say { $nodes{$h}{sock} } "configstr\n$configstr";
  SendCmd($rnodes, $h, "rank\n$nodes{$h}{rank}");
}

print "Launching...\n";
SendAllCmd($rnodes, "!run");
#foreach my $h (keys %nodes)
#{
#  SendCmd()
#  say { $nodes{$h}{sock} } "!run";
#}


my $live = scalar keys %nodes;
my $done = 0;
my $latest = 1e9;
print "Starting wait on $live nodes...\n";
until($done)
{
  $done = 1;
  while($live)
  {
    my @ready = $sel->can_read;
    foreach my $s (@ready)
    {
        $s->autoflush(1);
      $live--;
      print "Got response; now $live live\n";
      my $return = <$s>;
      chomp $return;
      if ($return == 500) {
          say "In barrier for MPI_Init";
          if ($live == 0) {
              $live = scalar keys %nodes;
              SendAllCmd($rnodes, "!com");
          }
          next;
      }
      my $lastepoch = <$s>;
      chomp $lastepoch;
      my $r = <$s>;
      chomp $r;
      if($lastepoch < $latest)
      {
        print "Updating to epoch $lastepoch\n";
        $latest = $lastepoch;
      }
      if($return == 0)
      {
        print "Got OK return\n";
      }
      else
      {
          #$done = 0;
        $live++;
        my $sig = $return & 127;
        my $st = $return >> 8;
        $epoch = $latest;
        print "Bad return!\n";
        print "\t Signal $sig\n";
        print "\t Status $st\n";
        print "\t Rank $r\n";
          
        print "Relaunching process...\n";
        #SendAllCmd($rnodes, "!kill");
        foreach my $h (keys %nodes)
        {
            #say { $nodes{$h}{sock} } "!kill";
            if ($nodes{$h}{rank} == $r) {
                SendCmd($rnodes, $h, "!run");
                last;
            }
        }
      }
    }
  }
    #unless($done)
    #{
    #$live = scalar keys %nodes;
    #$epoch = $latest;
    #$latest = 1e9;
    #say "Not done, so relaunching all in epoch $epoch";
    #SendAllCmd($rnodes, "epoch\n$epoch\n!run");
    #foreach my $h (keys %nodes)
    #{
    #  say { $nodes{$h}{sock} } "epoch\n$epoch\n!run";
    #
    #}
}

SendAllCmd($rnodes, "!done");
print "Finished!  Exiting...\n";

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
    if ($str =~ m/bin/) {
        $str .= "%" . $$hr{$haddr}{rank};
    }
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
my $numArgs = $#ARGV;
my $argstr;
if ($numArgs > 1) {
    $argstr = join(";", @ARGV[1..$#ARGV]);
}
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
SendAllCmd($rnodes, "bin\n$bin%$epoch%$argstr");
#foreach my $h (keys %nodes)
#{
  #say { $nodes{$h}{sock} } "bin\n$bin";
  #say { $nodes{$h}{sock} } "rank\n$nodes{$h}{rank}";
  #say { $nodes{$h}{sock} } "epoch\n$epoch";
  #say { $nodes{$h}{sock} } "configstr\n$configstr";
  #SendCmd($rnodes, $h, "rank\n$nodes{$h}{rank}");
#}

#print "Launching...\n";
#SendAllCmd($rnodes, "!run");
#foreach my $h (keys %nodes)
#{
#  SendCmd()
#  say { $nodes{$h}{sock} } "!run";
#}


my $live = scalar keys %nodes;
my $done = 0;
my $isAbort = 1;
my $latest = 1e9;
my $failed;
my $lowestAgreement;
my $isError = 0;

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
      if ($return eq "ack") {
      	if ($live == 0) {
      		$live = scalar keys %nodes;
      		print "Relaunching process...\n";
      		foreach my $h (keys %nodes) {
      			if ($nodes{$h}{rank} == $failed) {
      				SendCmd($rnodes, $h, "!run$lowestAgreement");
      			}
      		}
      	}
      next;
      }
     elsif ($return =~ m/nak/) {
     	my $ep = substr $return, 4;
     	if ($ep > $lowestAgreement) {
     		$lowestAgreement = $ep;
     	}
     }
      elsif ($return eq "barrier") {
          say "In barrier for MPI_Init";
          if ($isError) {
          	if ($live == ((scalar keys %nodes) - 1)) {
          		foreach my $h (keys %nodes) {
          			if ($nodes{$h}{rank} != $failed) {
          				SendCmd($rnodes, $h, "!commit$lowestAgreement");
          			}
          			else {
          				SendCmd($rnodes, $h, "!com");
          			}
          		}
          		$isError = 0;
          		$live = scalar keys %nodes;
          	}
          }
          else {
          	if ($live == 0) {
              $live = scalar keys %nodes;
              SendAllCmd($rnodes, "!com");
            }
          }
          
          next;
      }
        elsif ($return eq "run") {
            if ($live == 0) {
                $live = scalar keys %nodes;
                SendAllCmd($rnodes, "!run");
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
	# TODO how does this work???
        my $sig = $return & 127;
        my $st = $return >> 8;
        $epoch = $latest;
        print "Bad return!\n";
        print "\t Signal $sig\n";
        print "\t Status $st\n";
        print "\t Rank $r\n";
        $failed = $r;
          if ($st == 255) {
		# THIS IS MPI_ABORT
          	 # MPI Stages and Reinit
              $isAbort = 0;
              $live = 0;
              SendAllCmd($rnodes, "!kill");
             # End MPI Stages and Reinit
             
             # Checkpoint/restart
             #$live--;
             #foreach my $h (keys %nodes) {
             #	if ($nodes{$h}{rank} != $r) {
             #		SendCmd($rnodes, $h, "!kill");
             #	}
             #}
             # End Checkpoint/restart
          }
          else {
              print "Generating Consensus...\n";
              #SendAllCmd($rnodes, "!kill");
            
              foreach my $h (keys %nodes)
              {
                  #say { $nodes{$h}{sock} } "!kill";
                  if ($nodes{$h}{rank} != $r) {
                      SendCmd($rnodes, $h, "!err$latest");
                      #last;
                  }
              }
             $lowestAgreement = $latest;
             $live = (scalar keys %nodes) - 1;
             print "Waiting for Agreement...\n";
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
if ($isAbort) {
    SendAllCmd($rnodes, "!done");
}

print "Finished!  Exiting...\n";

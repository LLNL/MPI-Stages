#!/usr/bin/perl
# mpisrv.pl
# listening server for mpirun

use v5.10;
use strict;
use warnings;
use IO::Select;
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

my $sel = IO::Select->new();

die "Couldn't create listening socket; $!" unless $listen;

while(1)
{
  my $done = 0;
  say "Waiting for mpirun...";
  my $incoming = $listen->accept();
  $sel->add($incoming);
  say "Session live!";
  my %state;

  until($done)
  {
    my @ready =  $sel->can_read;
    say "Incoming...";
    foreach my $s (@ready)
    {
      my $cmd = <$s>;
      chomp $cmd;
      say "Got command $cmd";
      # test for special commands
      if($cmd eq "!run")
      {
        say "Launching!";
        my $pid = open(my $fh, "-|");
        die "Couldn't fork; game over" unless defined $pid;
        if($pid) # parent
        {
          $sel->add($fh);
          # have to remember these in case of kill
          $state{pid} = $pid;
          $state{fh} = $fh;
        }
        else #child, get the game on
        {
          my $configname = "mpirun.$state{rank}.config.tmp";
          my $epochname = "mpirun.$state{rank}.epoch.tmp"
          open(my $config, ">", $configname) or die "Couldn't create temporary file";
          print $config join("\n", split(/;/, $state{configstr}));
          my @args = ("$configname", "$state{rank}", "$epochname", "$state{epoch}", "$state{$bin}");
          push(@args, @ARGV);
          say "Launching $state{bin} as rank $state{rank}";
          my $result = system $bin @args;
          say $fh "!term";
          say $fh $result;
          close($fh);
          exit(0);
        }
      }
      elsif($cmd eq "!term")
      {
        my $result = <$s>;
        chomp $result;
        say "Execution completed with result $result";
        $sel->remove($s);
        close $s;
        my $configname = "mpirun.$state{rank}.config.tmp";
        open(my $epochfh, "<", $epochname);
        my $lastepoch = <$epochfh>;
        chomp $lastepoch;
        say $incoming $result;
        say $incoming $lastepoch;
        close $epochfh;
      }
      elsif($cmd eq "!kill")
      {
        # whatever you say
        say "Got request to terminate program :(";
        kill 9 $state{pid};
        $sel->remove{fh};
        close $state{pid};
      }
      elsif($cmd eq "!done")
      {
        # k
        $done = 1;
        close $incoming;
      }
      else  # just do a state update
      {
        my $state{$cmd} = <$s>;
        chomp $state{$cmd};
        say "Got update $cmd = $state{$cmd}";
      }
    } #until done
    say "All done with this round";
  }
  say "Done!";
}

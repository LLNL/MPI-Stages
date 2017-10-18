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

my $pid;
my $childfh;
my $parentfh;
my %state;
my $done;

while(1)  # main serve loop
{
  my $done = 0;
  say "Waiting for mpirun...";
  my $incoming = $listen->accept();
  $sel->add($incoming);
  say "Session live!";

  until($done)
  {
    my @ready =  $sel->can_read;
    say "Incoming...";
    foreach my $s (@ready)
    {
      #while(my $cmd = <$s>)
      my $cmd = <$s>;
      {
        die "Got nothing from socket" unless defined $cmd;
        chomp $cmd;
        say "Got command $cmd";
        # test for special commands
        if($cmd eq "!run")
        {
          say "Launching!";
          socketpair($childfh, $parentfh, AF_UNIX, SOCK_STREAM, PF_UNSPEC)
            or die "Couldn't socketpair ($!); fatal";
          $childfh->autoflush(1);
          $parentfh->autoflush(1);
          if($pid = fork())  # parent
          {
            close $parentfh;
            $sel->add($childfh);
            say "Launched (as parent), continuing...";
          }
          else  # child
          {
            close $childfh;

            my $configname = "mpirun.$state{rank}.config.tmp";
            my $epochname = "mpirun.$state{rank}.epoch.tmp";
            open(my $config, ">", $configname) or die "Couldn't create temporary file";
            print $config join("\n", split(/;/, $state{configstr}));
            close $config;
            open(my $epochconfig, ">", $epochname) or die "Couldn't create temporary epoch file";
            print $epochconfig "0\n"; 
            close $config;

            my @args = ("$configname", "$state{rank}", "$epochname", "$state{epoch}", "$state{bin}");
            push(@args, @ARGV);
            say "Launching $state{bin} as rank $state{rank}";
            my $bin = $state{bin};  # can't use hash in system args??
            my $result = system $bin @args;
            say "Finished $state{bin}, result = $result";
            say $parentfh "!term";
            say $parentfh $result;
            close($parentfh);
            say "Sent !term as child, exiting...";
            exit(0);
          }
        }
        elsif($cmd eq "!term")
        {
          my $result = <$s>;
          chomp $result;
          say "Execution completed with result $result";
          $sel->remove($childfh);
          close $childfh;
          my $configname = "mpirun.$state{rank}.config.tmp";
          my $epochname = "mpirun.$state{rank}.epoch.tmp";
          open(my $epochfh, "<", $epochname);
          my $lastepoch = <$epochfh>;
          chomp $lastepoch;
          $state{epoch} = $lastepoch;
          say "Got last valid epoch:  $lastepoch";
          say $incoming $result;
          say $incoming $lastepoch;
          say $incoming $state{rank};
          close $epochfh;
          waitpid($pid, 0);
        }
          #elsif($cmd eq "!kill")
          #{
          # whatever you say
          #say "Got request to terminate program :(";
          #kill 9, $pid;
          #my $result = waitpid($pid, 0);
          #$sel->remove($childfh);
          #my $epochname = "mpirun.$state{rank}.epoch.tmp";
          #open(my $epochfh, "<", $epochname) or die "No epoch file?  $!";
          #my $lastepoch = <$epochfh>;
          #chomp $lastepoch;
          #say "Got last valid epoch:  $lastepoch";
          #say $incoming $result;
          #say $incoming $lastepoch;
          #close $epochfh;
          #}
        elsif($cmd eq "!done")
        {
          # k
          $done = 1;
          close $incoming;
        }
        else  # just do a state update
        {
          $state{$cmd} = <$s>;
          chomp $state{$cmd};
          say "Got update $cmd = $state{$cmd}";
        }
        say "Done with cmd, about to read from socket again...";
      }
      say "About to read from next socket ...";
    } #until done
    say "\tFinished set, about to wait on sockets again";
  }
  $sel->remove($incoming);
  close($incoming);
  say "Done!";
}

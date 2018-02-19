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
my %state;
my $incoming;
my $childpid;

$SIG{USR1} = \&sigusr1_handler;

sub sigusr1_handler {
    say "USR1 signal received";
    
    print "From rank $state{rank}\n";
    
    my $pidfile = "pid.$state{rank}.txt";
    open(my $pidfh, "<", $pidfile) or die "Can't read pid";
    $childpid = <$pidfh>;
    chomp $childpid;
    
    my $barrier = 500;
    print $incoming "$barrier\n";
}

my $listen = new IO::Socket::INET (
  LocalHost => '0.0.0.0',
  LocalPort => $MPISOCKET,
  Proto => 'tcp',
  Listen => 5,
  Reuse => 1
);
$listen->autoflush(1);
my $sel = IO::Select->new();
die "Couldn't create listening socket; $!" unless $listen;

my $pid;
my $childfh;
my $parentfh;

my $done;

while(1)  # main serve loop
{
  my $done = 0;
  print "Waiting for mpirun...\n";
  $incoming = $listen->accept();
  $incoming->autoflush(1);
  $sel->add($incoming);
  print "Session live!\n";

  until($done)
  {
    my @ready =  $sel->can_read;
    print "Incoming...\n";
    foreach my $s (@ready)
    {
      $s->autoflush(1);
      #while(my $cmd = <$s>)
      my $cmd = <$s>;
      {
        die "Got nothing from socket" unless defined $cmd;
        chomp $cmd;
        print "Got command $cmd\n";
          my $parentpid = $$;
        # test for special commands
        if ($cmd eq "!com") {
            kill USR1 => $childpid;
        }
        elsif ($cmd eq "!err")
        {
            kill USR2 => $childpid;
        }
        elsif($cmd eq "!run")
        {
          print "Launching!\n";
          socketpair($childfh, $parentfh, AF_UNIX, SOCK_STREAM, PF_UNSPEC)
            or die "Couldn't socketpair ($!); fatal";
          $childfh->autoflush(1);
          $parentfh->autoflush(1);
          if($pid = fork())  # parent
          {
            close $parentfh;
            $sel->add($childfh);
            print "Launched (as parent), continuing...\n";
          }
          else  # child
          {
            close $childfh;

            my $configname = "mpirun.$state{rank}.config.tmp";
            my $epochname = "mpirun.$state{rank}.epoch.tmp";
            open(my $config, ">", $configname) or die "Couldn't create temporary file";
            $config->autoflush(1);
            print $config "ppid:$parentpid\n" . join("\n", split(/;/, $state{configstr}));
            close $config;
            open(my $epochconfig, ">", $epochname) or die "Couldn't create temporary epoch file";
            $epochconfig->autoflush(1);
            print $epochconfig "0\n"; 
            close $config;

            my @args = ("$configname", "$state{rank}", "$epochname", "$state{epoch}", "$state{bin}");
            push(@args, @ARGV);
            print "Launching $state{bin} as rank $state{rank}\n";
            my $bin = $state{bin};  # can't use hash in system args??
            my $result = system $bin @args;
            print "Finished $state{bin}, result = $result\n";
            print $parentfh "!term\n";
            print $parentfh "$result\n";
            close($parentfh);
            print "Sent !term as child, exiting...\n";
            exit(0);
          }
        }
        elsif($cmd eq "!term")
        {
          my $result = <$s>;
          chomp $result;
          print "Execution completed with result $result\n";
          $sel->remove($childfh);
          close $childfh;
          my $configname = "mpirun.$state{rank}.config.tmp";
          my $epochname = "mpirun.$state{rank}.epoch.tmp";
          open(my $epochfh, "<", $epochname);
          $epochfh->autoflush(1);
          my $lastepoch = <$epochfh>;
          chomp $lastepoch;
          $state{epoch} = $lastepoch;
          print "Got last valid epoch:  $lastepoch\n";
          print $incoming "$result\n";
          print $incoming "$lastepoch\n";
          print $incoming "$state{rank}\n";
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
          print "Got update $cmd = $state{$cmd}\n";
        }
        print "Done with cmd, about to read from socket again...\n";
      }
      print "About to read from next socket ...\n";
    } #until done
    print "\tFinished set, about to wait on sockets again\n";
  }
  $sel->remove($incoming);
  close($incoming);
  print "Done!\n";
}

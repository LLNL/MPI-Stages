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
my $isError = 0;
my $agreement;

my $configstr;
my $pid;
my $childfh;
my $parentfh;
my $lepoch;
my $done;

$SIG{USR1} = \&sigusr1_handler;

sub sigusr1_handler {
    say "USR1 signal received";   
    print "From rank $state{rank}\n";
    
    my $pidfile = "pid.$state{rank}.txt";
    open(my $pidfh, "<", $pidfile) or die "Can't read pid";
    $pidfh->autoflush(1);

	# this is in response to err and signal usr2
    if ($isError) {
    	my @lines = <$pidfh>;
    	chomp @lines;
    	$childpid = $lines[0];
    	chomp $childpid;
    	$agreement = $lines[1];
    	chomp $agreement;
    	close $pidfh;
    	if ($agreement == $lepoch) {
    		print $incoming "ack\n";
    	}
    	else {
    		print $incoming "nak$agreement\n";
    	}
    }
   else {
	# replace this with socket receive...
   		$childpid = <$pidfh>;
    	chomp $childpid;
    	close $pidfh;
    	print $incoming "barrier\n";
   } 
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
        if ($cmd =~ m/commit/) {
        	$lepoch = substr $cmd, 7;
        	my $epochname = "mpirun.$state{rank}.epoch.tmp";
            open(my $epochconfig, ">", $epochname) or die "Couldn't create temporary epoch file";
            $epochconfig->autoflush(1);
            print $epochconfig "$lepoch\n";
            close $epochconfig;
		# sending USR1
            kill USR1 => $childpid;
        }
        elsif ($cmd eq "!com") {
            kill USR1 => $childpid;
        }
        elsif ($cmd =~ m/!err/)
        {
        	$isError = 1;
            kill USR2 => $childpid;
            $lepoch = substr $cmd, 4;
            
        }
        elsif($cmd =~ m/!run/)
        {
          $lepoch = substr $cmd, 4;
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
            setpgrp(0, 0);
            my $configname = "mpirun.$state{rank}.config.tmp";
            my $epochname = "mpirun.$state{rank}.epoch.tmp";

			# write config file
            open(my $config, ">", $configname) or die "Couldn't create temporary file";
            $config->autoflush(1);
            print $config "ppid:$parentpid\n" . join("\n", split(/;/, $configstr));
            close $config;

              if ($state{epoch} == 0) {
                  open(my $epochconfig, ">", $epochname) or die "Couldn't create temporary epoch file";
                  $epochconfig->autoflush(1);
                  print $epochconfig "$state{epoch}\n";
                  close $epochconfig;
              }
              else {
                  open(my $epochconfig, ">", $epochname) or die "Couldn't create temporary epoch file";
                  $epochconfig->autoflush(1);
                  print $epochconfig "$lepoch\n";
                  $state{epoch} = $lepoch;
                  close $epochconfig;
              }

            my @args = ("$configname", "$state{rank}", "$epochname", "$state{epoch}", "$state{bin}");
            my @arglist = split(/;/, $state{argstr});
            push(@args, @arglist);
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
          elsif($cmd eq "!kill")
          {
          	 # MPI Stages and Reinit
              close $incoming;
              kill -9, $pid;
              exit(0);
             # End MPI Stages and Reinit
             
             # Checkpoint/Restart
             #kill USR2 => $childpid;
             # End Checkpoint/restart
             
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
          }
        elsif($cmd eq "!done")
        {
          # k
          $done = 1;
          close $incoming;
        }
        else  # just do a state update
        {
            my $status = <$s>;
            chomp $status;
            if ($status =~ m/%/) {
                #my $filename;
                my @statuslist = split(/%/, $status);
                $state{"bin"} = $statuslist[0];
                $state{"epoch"} = $statuslist[1];
                $state{"argstr"} = $statuslist[2];
                $state{"rank"} = $statuslist[3];
                #$filename = $statuslist[4];
                
                open(my $hostsfh, "<", "MPIHOSTS") or die "Can't read MPIHOSTS";
                my @hosts = <$hostsfh>;
                chomp @hosts;
                my $sz = scalar @hosts;
                my $nextrank = 0;
                my $host;
                my %nodes;
                

				# create config?
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
                $configstr = join(";",@configlist);
            }
          
          print "Got update $cmd = $state{$cmd}\n";
          print $incoming "run\n";
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

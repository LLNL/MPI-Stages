# mpirun.pl
# Initiate MPI launch to hosts specified in MPIHOSTS

use v5.10;
use strict;
use warnings;
use IO::Socket::INET;

my ($bin) = @ARGV;
my $MPISOCKET = 4422;

open(my $hostsfh, "<", "MPIHOSTS") or die "Can't read MPIHOSTS";
my @hosts = <$hostsfh>;
my $sz = $hosts;
my $nextrank = 1;

foreach $host (@hosts)
{
  say "Assigning rank $nextrank to $host...";
  my $s = new IO::Socket::INET(
    PeerHost => $host,
    PeerPort => $MPISOCKET,
    Proto => 'tcp',
  );

  die "Failed connecting to $host:$MPISOCKET..." unless $s;
  $s->send($bin);
  $s->send($nextrank);
  $s->send($sz);
  $s->send(join("\n", @hosts));
  $nextrank += 1;
  $s->close();
}

say "Done!";

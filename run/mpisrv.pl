# mpisrv.pl
# listening server for mpirun

use v5.10;
use strict;
use warnings;
use IO::Socket::INET;
$| = 1;
 
my $MPISOCKET 4422;

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
  my $bin = "";
  my $rank = "";
  $s->recv($bin, 2048);
  $s->recv($rank, 16);
  say "Launching $bin as rank $rank";
  say `$bin MPIARGST $rank 0 MPIARGEN`;
  shutdown($s, 1);
}

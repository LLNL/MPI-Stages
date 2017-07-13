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
  my $sz = "";
  my $hosts = "";
  $s->recv($bin, 2048);
  $s->recv($rank, 16);
  $s->recv($sz, 16);
  $s->recv($hosts, 1024 * 16);
  open(my $hostsfile, ">", "mpihosts.stdin.tmp") or die "Couldn't create temporary stdin file";
  print $hostsfile $hosts;
  say "Launching $bin as rank $rank";
  say `$bin MPIARGST $rank $sz MPIARGEN < mpihosts.stdin.tmp`;
  shutdown($s, 1);
}

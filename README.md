# ExaMPI
UTC-Auburn-LLNL Next-Generation MPI Implementation

**Warning:  master is not yet stable**

Compile instruction - 
1. cd AUMPI
2. make

Run instruction - 
1. cd AUMPI/run
2. Edit MPIHOSTS to add node's IP.
3. Copy the binary hello from AUMPI/examples/hello to AUMPI/run
4. Run mpisrv.pl on each node
   -> ./mpisrv.pl
5. Run mpirun.pl on the launch node with the binary
   -> ./mpirun.pl hello

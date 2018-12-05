#! /bin/bash

# check for running, exit if so
script_name=$(basename "$0")
this_script=$(basename "${BASH_SOURCE[0]}")
if [ $script_name = $this_script ]; then
	echo "source $script_name"
	exit 
fi

basepath="$(dirname "$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)")"

binpath="$basepath/bin"
libpath="$basepath/lib"
incpath="$basepath/include"
runpath="$basepath/runtime"

# PATH
export PATH=$PATH:$binpath
export PATH=$PATH:$runpath

# LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$libpath

export MPI_PATH=$binpath
export MPI_LIB=$libpath
export MPI_INC=$incpath
export MPI_HOME=$basepath

#!/bin/bash
procesos=4
debug=0
executable="simulation"

if [ "$#" -ge 1 ]; then
	procesos=$1
fi
if [ "$#" -ge 2 ]; then
	debug=$2
fi

if [ "$debug" -eq 1 ]; then
	mpirun -v -np $procesos $executable
else
	mpirun -np $procesos $executable
fi

#!/bin/bash
PROCESS=4
THREADS=8
EXECUTABLE="simulation"
TEMPDIRECTORY="temp"

if [ "$#" -ge 1 ]; then
	PROCESS=$1
fi

if [ "$#" -ge 2 ]; then
	THREADS=$2
fi

# Verificar si la carpeta 'temp' existe
if [ -d "$TEMPDIRECTORY" ]; then
    # Borrar la carpeta 'temp' y su contenido
    rm -r "$TEMPDIRECTORY"
    echo "Carpeta $TEMPDIRECTORY y su contenido eliminados."
fi

mkdir "$TEMPDIRECTORY"
echo "Carpeta $TEMPDIRECTORY creada de nuevo."
make clean
make
mpirun -np $PROCESS $EXECUTABLE $THREADS

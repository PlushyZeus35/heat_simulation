#!/bin/bash
THREADS=8
EXECUTABLE="simulation"
TEMPDIRECTORY="temp"

if [ "$#" -ge 1 ]; then
	THREADS=$1
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
./$EXECUTABLE $THREADS

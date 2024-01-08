#!/bin/bash
THREADS=4
EXECUTIONFILE="simulation"
TEMPDIRECTORY="temp"
PYTHONCMD="python"
PYTHONSCRIPT="doGif.py"

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
./$EXECUTIONFILE $THREADS

echo "Intentando generar el gif."
if command -v python3 &>/dev/null; then
    PYTHONCMD=python3
elif command -v python &>/dev/null; then
    PYTHONCMD=python
else
    echo "No se encontró Python en el sistema."
    exit 1
fi

echo "Ejecutando el script con $PYTHONCMD"

$PYTHONCMD $PYTHONSCRIPT 

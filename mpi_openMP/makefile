# Nombre del archivo ejecutable
TARGET = simulation

# Fuentes
SRCS = heat_mpi_omp.c mpiUtils.c simulationUtils.c logUtils.c pngwriter.c constants.h

# Opciones de compilación
CFLAGS = -Wall -Wextra -lpng -fopenmp

# Regla por defecto
all: $(TARGET)

# Regla para compilar el programa
$(TARGET):	$(SRCS)
	mpicc $^ -o $@ $(CFLAGS)

# Limpieza
clean:
	rm -f $(TARGET)

.PHONY: all clean

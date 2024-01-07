# Nombre del archivo ejecutable
TARGET = simulation

# Fuentes
SRCS = lineal.c simulationUtils.c logUtils.c pngwriter.c constants.h

# Opciones de compilación
CFLAGS = -Wall -Wextra -lpng

# Regla por defecto
all: $(TARGET)

# Regla para compilar el programa
$(TARGET):	$(SRCS)
	gcc $^ -o $@ $(CFLAGS)

# Limpieza
clean:
	rm -f $(TARGET)

.PHONY: all clean

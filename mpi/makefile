# Nombre del archivo ejecutable
TARGET = simulation

# Fuentes
SRCS = heat_lineal_formatted.c pngwriter.c

# Opciones de compilación
CFLAGS = -Wall -Wextra -lpng -lpthread

# Regla por defecto
all: $(TARGET)

# Regla para compilar el programa
$(TARGET):	$(SRCS)
	gcc $^ -o $@ $(CFLAGS)

# Limpieza
clean:
	rm -f $(TARGET)

.PHONY: all clean

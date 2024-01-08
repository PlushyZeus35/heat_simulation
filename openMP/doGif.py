import os
from PIL import Image
import imageio

# Establece la carpeta donde se encuentran tus imágenes
carpeta_imagenes = './temp/'

# Obtiene una lista de archivos de imagen en la carpeta
archivos_imagen = [os.path.join(carpeta_imagenes, nombre_archivo) for nombre_archivo in os.listdir(carpeta_imagenes) if nombre_archivo.endswith('.png')]

# Ordena los archivos de imagen por nombre (si es necesario)
archivos_imagen.sort()

# Crea una lista para almacenar los frames del GIF
frames = []

# Recorre los archivos de imagen y agrega cada imagen como un frame
for archivo_imagen in archivos_imagen:
    img = Image.open(archivo_imagen)
    frames.append(img)

# Establece la ruta de salida para el GIF
gif_salida = 'heat.gif'

# Guarda los frames como un GIF usando imageio
imageio.mimsave(gif_salida, frames, duration=0.5, loop=0)  # Puedes ajustar la duración entre los frames

print(f'GIF guardado en {gif_salida}')

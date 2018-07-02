# Requerimientos - Iteración #1
---
> 01 de Julio del 2018

La iteración núcleo debe complir los siguientes requerimientos:

  *  En cuanto a la compresión:
      *  Comprimir con cada algoritmo disponible (ZLIB, Snappy, LZO, Bzip2, LZMA, FZP, copia).
          *  Cada método de compresión debe poder comprimir flujos de datos en lugar de archivos.
      *  Un método genérico o wrapper para comprimir con los siguientes parámetros:
          *  Método de compresión.
          *  Buffers de entrada y salida.
          *  Tamaños de los buffers de entrada y salida.
          *  Nivel de compresión (cuando sea necesario).
  *  Infraestructura para programas de envío y recepción de mensajes:
      *  Se debe considerar la compresión y transmisión simultáneas.
      *  Se deben comprimir los mensajes que llegan en modalidad round robin.
      *  El programa puede verse como un demonio que se encuentra en un computador remoto, en el cual se encuentran los archivos que se quieren transmitir. El cliente tiene la dirección (IP+puerto) del servidor y envía la ruta absoluta de los archivos que desea descargar. El servidor recibe una petición y transmite al cliente los archivos solicitados, comprimiendo según el ítem b. Se puede pensar como en una especie de scp.
  *  El resto de los requerimientos son recogidos en reuniones o entrevistas con el usuario (tutor).

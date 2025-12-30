# Caracteristicas
- Control de 4 servos independientes mediante joysticks virtuales personalizados

- Comunicación en tiempo real usando WebSockets para baja latencia

- Interfaz responsiva que funciona en dispositivos móviles

- Modos de operación: Manual y Giro controlado

- Sistema de notificaciones visuales al usuario

- Indicadores de conexión en tiempo real

- Prevención de conflictos entre comandos simultáneos

- Simulación completa para demostración sin hardware

# Tecnologías Utilizadas
- Frontend: HTML, CSS y JavaScript 

- Comunicación: WebSocket (protocolo WSS/WS)

- Control de servos: Biblioteca ESP32Servo

- Red: WiFi en modo Access Point (AP)

- Servidor Web: WebServer para ESP32

# Archivos Principales del Proyecto:
 # index.html - Interfaz web completa con:
   - 4 joysticks virtuales independientes (2 verticales, 2 horizontales)
      
   - Panel de control central con botones de acción

Sistema de notificaciones (toasts)

Indicadores de estado y conexión

Modo demostración integrado

main.cpp - Código ESP32 con:

Configuración de Access Point WiFi

Servidor WebSocket en puerto 81

Control de 4 servos motores

Lógica de giro único con temporizador

Manejo de comandos desde la web

platformio.ini - Configuración de PlatformIO para:

Compilación y despliegue en ESP32

Dependencias de bibliotecas (WebSockets, ESP32Servo)

Configuración de puertos y velocidad

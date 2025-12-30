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

# Archivos Principales del Proyecto
 # index.html - Interfaz web completa con:
   - 4 joysticks virtuales independientes (2 verticales, 2 horizontales)
   - Panel de control central con botones de acción
   - Sistema de notificaciones 
   - Indicadores de estado y conexión
   - Modo demostración integrado

 # main.cpp - Código ESP32 con:
   - Configuración de Access Point WiFi
   - Servidor WebSocket en puerto 81
   - Control de 4 servos motores
   - Lógica de giro único con temporizador
   - Manejo de comandos desde la web

 # platformio.ini - Configuración de PlatformIO para:
   - Compilación y despliegue en ESP32
   - Dependencias de bibliotecas (WebSockets, ESP32Servo)
   - Configuración de puertos y velocidad

# Funcionalidades Principales
1. Sistema de Joysticks Virtuales
 - Joystick Izquierdo - Vertical: Controla Servo X (movimiento arriba/abajo)

 - Joystick Izquierdo - Horizontal: Controla Servo Y (movimiento izquierda/derecha)

 - Joystick Derecho - Vertical: Controla Servo A (movimiento adicional)

 - Joystick Derecho - Horizontal: Controla Servo B (servo extra)

 - Cada joystick convierte el arrastre del usuario en valores angulares (60°-120°) que se envían al ESP32.

2. Comunicación WebSocket
 - Conexión bidireccional en tiempo real

 - Comandos simples tipo X:90, Y:120, A:60, B:90

 - Eventos de sistema: GIRO:INICIAR, GIRO:CANCELAR

 - Feedback del estado: GIRO:INICIADO, GIRO:COMPLETADO

3. Control Central
 - Centrar Servos: Botón para regresar todos los servos a posición neutral (90°)

 - Giro Controlado: Sistema de giro único de 1 segundo con cancelación

 - Prevención de conflictos: Bloqueo de joysticks durante giros activos

4. Interfaz de Usuario
 - Diseño moderno con gradientes y efectos visuales

 - Responsive para móviles (media queries)

 - Sistema de notificaciones tipo "toast"

 - Indicadores visuales de estado de conexión

 - Feedback táctil y visual en tiempo real

# Componentes Clave
Clase DroneController (JavaScript):
 - Gestión de conexión WebSocket
 - Control de eventos de joysticks
 - Manejo de estados de giro
 - Sistema de notificaciones
 - Prevención de comandos conflictivos

ESP32 (main.cpp):
 - Access Point: DroneRobot (contraseña: 12345678)
 - Servidor Web en puerto 80 (interfaz)
 - Servidor WebSocket en puerto 81 (comandos)
 - Control PWM de 4 servos (pines 2, 15, 13, 14)
 - Sistema de temporización para giros controlados

# Instalación y Uso
1. Configuración del Hardware:
// Pines de servos en ESP32
const int pinServoX = 2;   // Joystick Izquierdo Arriba/Abajo
const int pinServoY = 15;  // Joystick Izquierdo Izquierda/Derecha
const int pinServoA = 13;  // Joystick Derecho Arriba/Abajo
const int pinServoB = 14;  // Servo Extra

2. Despliegue Web:
 - Subir index.html a cualquier servidor web o GitHub Pages
 - Compilar y cargar main.cpp al ESP32 usando PlatformIO
 - Conectar al WiFi DroneRobot desde cualquier dispositivo
 - Acceder a http://192.168.4.1 desde el navegador

3. Modo Demostración:
El archivo index.html incluye un modo demostración completo que funciona sin ESP32, perfecto para portfolios y demostraciones online.

Protocolo de Comandos
Desde Web → ESP32:
X:[60-120] - Control Servo X

Y:[60-120] - Control Servo Y

A:[60-120] - Control Servo A

B:[60-120] - Control Servo B

GIRO:INICIAR - Inicia giro de 1 segundo

GIRO:CANCELAR - Cancela giro en curso

Desde ESP32 → Web:
GIRO:INICIADO - Confirmación de giro iniciado

GIRO:COMPLETADO - Notificación de giro finalizado

GIRO:CANCELADO - Confirmación de giro cancelado

MODO:MANUAL - Estado del modo operativo

# Valor del Proyecto
Este proyecto demuestra competencias en:

Desarrollo Full Stack: Integración de frontend web con hardware embebido

Comunicación en Tiempo Real: Implementación de WebSockets para baja latencia

Diseño de UI/UX: Creación de interfaces intuitivas y responsivas

Programación Embebida: Control de hardware con ESP32 y C++

Protocolos de Red: Configuración de Access Point y servidores web

Control de Sistemas: Manipulación precisa de servos motores

Gestión de Estados: Sistema robusto para prevenir conflictos de comandos

Ideal para aplicaciones de robótica, drones educativos, control remoto de dispositivos y demostraciones de IoT. La arquitectura modular permite fácil extensión para más servos, sensores o funcionalidades adicionales.

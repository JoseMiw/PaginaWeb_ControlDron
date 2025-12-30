#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>
#include <WebServer.h>

// --- Config WiFi ---
// Modo: AP (Access Point) - El ESP32 crea su propia red
const char* ssid_ap = "DroneRobot"; // Nombre de la red del drone
const char* password_ap = "12345678"; // Contraseña (mínimo 8 caracteres)

// --- Servos ---
Servo servoX;  // Servo X - Joystick Izquierdo Arriba/Abajo
Servo servoY;  // Servo Y - Joystick Izquierdo Izquierda/Derecha
Servo servoA;  // Servo A - Joystick Derecho Arriba/Abajo
Servo servoB;  // Servo B - Servo Extra

// Pines
const int pinServoX = 2;
const int pinServoY = 15;
const int pinServoA = 13;
const int pinServoB = 14;

// --- WebSocket y Web Server ---
WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);

// Variables para el giro único
bool giroActivo = false;
unsigned long tiempoInicioGiro = 0;
const unsigned long duracionGiro = 1000; // 1 segundo de giro
int anguloGiro = 120; // Ángulo para el giro

// Página Web mejorada con 4 joysticks - CORREGIDA
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Control Drone Robot</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
            color: #333;
        }

        .container {
            max-width: 1400px;
            margin: 0 auto;
        }

        .header {
            text-align: center;
            margin-bottom: 30px;
            color: white;
        }

        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }

        .header p {
            font-size: 1.1em;
            opacity: 0.9;
        }

        .status-bar {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 15px;
            padding: 15px 20px;
            margin-bottom: 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
            backdrop-filter: blur(10px);
        }

        .status-item {
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .status-dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            background: #4CAF50;
            animation: pulse 2s infinite;
        }

        .status-dot.offline {
            background: #f44336;
        }

        .control-panel {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-bottom: 20px;
        }

        @media (max-width: 768px) {
            .control-panel {
                grid-template-columns: 1fr;
            }
        }

        .joystick-section {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 25px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
            backdrop-filter: blur(10px);
        }

        .section-title {
            font-size: 1.4em;
            font-weight: 600;
            margin-bottom: 25px;
            color: #333;
            text-align: center;
            padding: 10px;
            border-radius: 10px;
            background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
        }

        .joystick-row {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-bottom: 20px;
        }

        .joystick-container {
            text-align: center;
        }

        .joystick-label {
            font-size: 1.1em;
            font-weight: 600;
            margin-bottom: 15px;
            color: #555;
        }

        .joystick {
            width: 150px;
            height: 150px;
            background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
            border-radius: 50%;
            margin: 0 auto;
            position: relative;
            border: 3px solid #ddd;
            box-shadow: inset 0 0 20px rgba(0,0,0,0.1);
            touch-action: none;
            cursor: pointer;
        }

        .joystick-vertical {
            width: 100px;
            height: 180px;
            border-radius: 50px;
        }

        .joystick-horizontal {
            width: 180px;
            height: 100px;
            border-radius: 50px;
        }

        .joystick-handle {
            width: 50px;
            height: 50px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            border-radius: 50%;
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
            transition: transform 0.1s ease;
        }

        .joystick-handle-vertical {
            width: 70px;
            height: 40px;
            border-radius: 20px;
        }

        .joystick-handle-horizontal {
            width: 40px;
            height: 70px;
            border-radius: 20px;
        }

        .servo-value {
            margin-top: 15px;
            font-size: 1.1em;
            font-weight: 600;
            color: #333;
            background: #f8f9fa;
            padding: 8px 15px;
            border-radius: 25px;
            display: inline-block;
        }

        .mode-control {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 25px;
            text-align: center;
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
            backdrop-filter: blur(10px);
            margin-bottom: 20px;
        }

        .mode-title {
            font-size: 1.3em;
            font-weight: 600;
            margin-bottom: 20px;
            color: #333;
        }

        .mode-buttons {
            display: flex;
            gap: 15px;
            justify-content: center;
            flex-wrap: wrap;
            margin-bottom: 20px;
        }

        .btn {
            padding: 12px 24px;
            border: none;
            border-radius: 25px;
            font-size: 1em;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            min-width: 120px;
        }

        .btn-primary {
            background: linear-gradient(135deg, #4CAF50, #45a049);
            color: white;
        }

        .btn-warning {
            background: linear-gradient(135deg, #FF9800, #F57C00);
            color: white;
        }

        .btn-danger {
            background: linear-gradient(135deg, #f44336, #d32f2f);
            color: white;
        }

        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.2);
        }

        .btn:active {
            transform: translateY(0);
        }

        .btn:disabled {
            opacity: 0.6;
            cursor: not-allowed;
            transform: none;
        }

        .current-mode {
            margin-top: 20px;
            padding: 15px;
            border-radius: 15px;
            background: #f8f9fa;
            font-weight: 600;
        }

        .mode-manual {
            background: #e3f2fd;
            color: #1565c0;
            border: 2px solid #2196F3;
        }

        .giro-control {
            margin: 20px 0;
            padding: 20px;
            background: #fff3e0;
            border-radius: 15px;
            border: 2px solid #FF9800;
        }

        .giro-title {
            font-size: 1.2em;
            font-weight: 600;
            margin-bottom: 15px;
            color: #e65100;
            text-align: center;
        }

        .giro-buttons {
            display: flex;
            gap: 10px;
            justify-content: center;
            flex-wrap: wrap;
        }

        .giro-status {
            margin-top: 15px;
            padding: 10px;
            border-radius: 10px;
            background: #ffecb3;
            text-align: center;
            font-weight: 600;
        }

        .giro-active {
            background: #c8e6c9;
            color: #2e7d32;
        }

        .giro-inactive {
            background: #ffcdd2;
            color: #c62828;
        }

        .instructions {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 25px;
            margin-top: 20px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
            backdrop-filter: blur(10px);
        }

        .instructions h3 {
            margin-bottom: 15px;
            color: #333;
        }

        .instructions ul {
            list-style: none;
            padding: 0;
        }

        .instructions li {
            padding: 8px 0;
            border-bottom: 1px solid #eee;
        }

        .instructions li:last-child {
            border-bottom: none;
        }

        .connection-status {
            display: flex;
            align-items: center;
            gap: 8px;
        }

        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }

        .toast {
            position: fixed;
            top: 20px;
            right: 20px;
            padding: 15px 25px;
            background: #333;
            color: white;
            border-radius: 10px;
            box-shadow: 0 5px 15px rgba(0,0,0,0.3);
            transform: translateX(400px);
            transition: transform 0.3s ease;
            z-index: 1000;
        }

        .toast.show {
            transform: translateX(0);
        }

        .toast.success {
            background: #4CAF50;
        }

        .toast.error {
            background: #f44336;
        }

        .toast.info {
            background: #2196F3;
        }

        .toast.warning {
            background: #FF9800;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🚁 Control Drone Robot</h1>
            <p>Controla los 4 servos del drone - 4 Joysticks Independientes</p>
        </div>

        <div class="status-bar">
            <div class="status-item">
                <div class="status-dot" id="connectionDot"></div>
                <span id="connectionStatus">Conectando...</span>
            </div>
            <div class="status-item">
                <span id="ipAddress">IP: 192.168.4.1</span>
            </div>
        </div>

        <div class="control-panel">
            <!-- Joystick Izquierdo -->
            <div class="joystick-section">
                <div class="section-title" style="background: linear-gradient(135deg, #FF6B6B, #FF8E8E); color: white;">
                    🎮 Joystick Izquierdo
                </div>
                
                <div class="joystick-row">
                    <div class="joystick-container">
                        <div class="joystick-label">Arriba/Abajo (Servo X)</div>
                        <div class="joystick joystick-vertical" id="joystickLeftUpDown">
                            <div class="joystick-handle joystick-handle-vertical"></div>
                        </div>
                        <div class="servo-value">Servo X: <span id="servoX">90°</span></div>
                    </div>
                    
                    <div class="joystick-container">
                        <div class="joystick-label">Izquierda/Derecha (Servo Y)</div>
                        <div class="joystick joystick-horizontal" id="joystickLeftLeftRight">
                            <div class="joystick-handle joystick-handle-horizontal"></div>
                        </div>
                        <div class="servo-value">Servo Y: <span id="servoY">90°</span></div>
                    </div>
                </div>
            </div>

            <!-- Joystick Derecho -->
            <div class="joystick-section">
                <div class="section-title" style="background: linear-gradient(135deg, #4ECDC4, #88D3CE); color: white;">
                    🎮 Joystick Derecho
                </div>
                
                <div class="joystick-row">
                    <div class="joystick-container">
                        <div class="joystick-label">Arriba/Abajo (Servo A)</div>
                        <div class="joystick joystick-vertical" id="joystickRightUpDown">
                            <div class="joystick-handle joystick-handle-vertical"></div>
                        </div>
                        <div class="servo-value">Servo A: <span id="servoA">90°</span></div>
                    </div>
                    
                    <div class="joystick-container">
                        <div class="joystick-label">Extra (Servo B)</div>
                        <div class="joystick joystick-horizontal" id="joystickRightLeftRight">
                            <div class="joystick-handle joystick-handle-horizontal"></div>
                        </div>
                        <div class="servo-value">Servo B: <span id="servoB">90°</span></div>
                    </div>
                </div>
            </div>
        </div>

        <div class="mode-control">
            <div class="mode-title">⚙️ Control Central</div>
            
            <div class="mode-buttons">
                <button class="btn btn-primary" onclick="centerAllServos()">🔄 Centrar Todos los Servos</button>
            </div>

            <div class="giro-control">
                <div class="giro-title">🔄 Control de Giro Único</div>
                <div class="giro-buttons">
                    <button class="btn btn-warning" onclick="iniciarGiro()" id="btnGiro">🔄 Iniciar Giro</button>
                    <button class="btn btn-danger" onclick="cancelarGiro()" id="btnCancelar">⏹️ Cancelar Giro</button>
                </div>
                <div class="giro-status giro-inactive" id="giroStatus">
                    Giro: INACTIVO
                </div>
            </div>
            
            <div class="current-mode mode-manual" id="currentMode">
                Modo Actual: MANUAL
            </div>
        </div>

        <div class="instructions">
            <h3>📋 Instrucciones de Uso</h3>
            <ul>
                <li>• <strong>Joystick Izquierdo - Arriba/Abajo:</strong> Controla el Servo X (60°-120°)</li>
                <li>• <strong>Joystick Izquierdo - Izquierda/Derecha:</strong> Controla el Servo Y (60°-120°)</li>
                <li>• <strong>Joystick Derecho - Arriba/Abajo:</strong> Controla el Servo A (60°-120°)</li>
                <li>• <strong>Joystick Derecho - Izquierda/Derecha:</strong> Controla el Servo B (60°-120°)</li>
                <li>• <strong>Botón Centro:</strong> Regresa todos los servos a posición central (90°)</li>
                <li>• <strong>Giro Único:</strong> Presiona "Iniciar Giro" para un giro controlado de 1 segundo</li>
                <li>• <strong>Cancelar Giro:</strong> Detiene el giro y vuelve al centro inmediatamente</li>
                <li>• <strong>Rango:</strong> Los servos se mueven entre 60° y 120° en ambas direcciones</li>
            </ul>
        </div>
    </div>

    <div class="toast" id="toast"></div>

    <script>
        class DroneController {
            constructor() {
                this.ws = null;
                this.isConnected = false;
                this.currentMode = 'MANUAL';
                this.giroActivo = false;
                this.activeJoysticks = new Set();
                this.initWebSocket();
                this.initJoysticks();
            }

            initWebSocket() {
                const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
                const wsUrl = `${protocol}//${window.location.hostname}:81`;
                
                this.ws = new WebSocket(wsUrl);
                
                this.ws.onopen = () => {
                    this.isConnected = true;
                    this.updateConnectionStatus(true);
                    this.showToast('✅ Conectado al Drone', 'success');
                };

                this.ws.onclose = () => {
                    this.isConnected = false;
                    this.updateConnectionStatus(false);
                    this.showToast('❌ Conexión perdida', 'error');
                    setTimeout(() => this.initWebSocket(), 3000);
                };

                this.ws.onmessage = (event) => {
                    const message = event.data;
                    if (message === 'GIRO:INICIADO') {
                        this.setGiroStatus(true);
                    } else if (message === 'GIRO:COMPLETADO') {
                        this.setGiroStatus(false);
                    } else if (message === 'GIRO:CANCELADO') {
                        this.setGiroStatus(false);
                    }
                };

                this.ws.onerror = (error) => {
                    console.error('WebSocket error:', error);
                    this.showToast('❌ Error de conexión', 'error');
                };
            }

            initJoysticks() {
                // Joystick Izquierdo - Arriba/Abajo (Servo X)
                this.setupVerticalJoystick('joystickLeftUpDown', (value) => {
                    if (this.giroActivo) return;
                    this.sendCommand(`X:${value}`);
                    document.getElementById('servoX').textContent = value + '°';
                });

                // Joystick Izquierdo - Izquierda/Derecha (Servo Y)
                this.setupHorizontalJoystick('joystickLeftLeftRight', (value) => {
                    if (this.giroActivo) return;
                    this.sendCommand(`Y:${value}`);
                    document.getElementById('servoY').textContent = value + '°';
                });

                // Joystick Derecho - Arriba/Abajo (Servo A)
                this.setupVerticalJoystick('joystickRightUpDown', (value) => {
                    if (this.giroActivo) return;
                    this.sendCommand(`A:${value}`);
                    document.getElementById('servoA').textContent = value + '°';
                });

                // Joystick Derecho - Izquierda/Derecha (Servo B)
                this.setupHorizontalJoystick('joystickRightLeftRight', (value) => {
                    if (this.giroActivo) return;
                    this.sendCommand(`B:${value}`);
                    document.getElementById('servoB').textContent = value + '°';
                });
            }

            setupVerticalJoystick(elementId, callback) {
                const joystick = document.getElementById(elementId);
                const handle = joystick.querySelector('.joystick-handle');
                const maxDistance = 65; // Limitación para movimiento vertical

                let isDragging = false;
                let joystickId = elementId;

                const getJoystickRect = () => {
                    return joystick.getBoundingClientRect();
                };

                const updatePosition = (clientY) => {
                    const rect = getJoystickRect();
                    const centerY = rect.height / 2;
                    const y = clientY - rect.top - centerY;
                    
                    // Aplicar límites de movimiento
                    const distance = Math.max(-maxDistance, Math.min(maxDistance, y));
                    
                    // Actualizar posición visual del handle
                    handle.style.transform = `translate(-50%, ${distance}px)`;
                    
                    // Convertir a valores entre 60-120 (90 ± 30)
                    // Arriba: 60° (mínimo), Centro: 90°, Abajo: 120° (máximo)
                    const joyValue = Math.round(90 + (distance / maxDistance) * 30);
                    callback(joyValue);
                };

                const startDrag = (clientY) => {
                    if (this.giroActivo) {
                        this.showToast('⏳ Giro en progreso - Espere...', 'warning');
                        return;
                    }
                    
                    if (this.activeJoysticks.has(joystickId)) {
                        return; // Ya está siendo arrastrado
                    }
                    
                    isDragging = true;
                    this.activeJoysticks.add(joystickId);
                    updatePosition(clientY);
                };

                const endDrag = () => {
                    if (!isDragging) return;
                    
                    isDragging = false;
                    this.activeJoysticks.delete(joystickId);
                    handle.style.transform = 'translate(-50%, -50%)';
                    callback(90); // Volver al centro
                };

                // Eventos táctiles
                const handleTouchStart = (e) => {
                    e.preventDefault();
                    startDrag(e.touches[0].clientY);
                };

                const handleTouchMove = (e) => {
                    e.preventDefault();
                    if (isDragging) {
                        updatePosition(e.touches[0].clientY);
                    }
                };

                const handleTouchEnd = (e) => {
                    e.preventDefault();
                    endDrag();
                };

                joystick.addEventListener('touchstart', handleTouchStart, { passive: false });
                joystick.addEventListener('touchmove', handleTouchMove, { passive: false });
                joystick.addEventListener('touchend', handleTouchEnd);
                joystick.addEventListener('touchcancel', handleTouchEnd);

                // Eventos mouse
                const handleMouseDown = (e) => {
                    startDrag(e.clientY);
                };

                const handleMouseMove = (e) => {
                    if (isDragging) {
                        updatePosition(e.clientY);
                    }
                };

                const handleMouseUp = () => {
                    endDrag();
                };

                joystick.addEventListener('mousedown', handleMouseDown);
                document.addEventListener('mousemove', handleMouseMove);
                document.addEventListener('mouseup', handleMouseUp);

                // Prevenir comportamiento por defecto
                joystick.addEventListener('dragstart', (e) => e.preventDefault());
                joystick.addEventListener('contextmenu', (e) => e.preventDefault());
            }

            setupHorizontalJoystick(elementId, callback) {
                const joystick = document.getElementById(elementId);
                const handle = joystick.querySelector('.joystick-handle');
                const maxDistance = 65; // Limitación para movimiento horizontal

                let isDragging = false;
                let joystickId = elementId;

                const getJoystickRect = () => {
                    return joystick.getBoundingClientRect();
                };

                const updatePosition = (clientX) => {
                    const rect = getJoystickRect();
                    const centerX = rect.width / 2;
                    const x = clientX - rect.left - centerX;
                    
                    // Aplicar límites de movimiento
                    const distance = Math.max(-maxDistance, Math.min(maxDistance, x));
                    
                    // Actualizar posición visual del handle
                    handle.style.transform = `translate(${distance}px, -50%)`;
                    
                    // Convertir a valores entre 60-120 (90 ± 30)
                    // Izquierda: 60° (mínimo), Centro: 90°, Derecha: 120° (máximo)
                    const joyValue = Math.round(90 + (distance / maxDistance) * 30);
                    callback(joyValue);
                };

                const startDrag = (clientX) => {
                    if (this.giroActivo) {
                        this.showToast('⏳ Giro en progreso - Espere...', 'warning');
                        return;
                    }
                    
                    if (this.activeJoysticks.has(joystickId)) {
                        return; // Ya está siendo arrastrado
                    }
                    
                    isDragging = true;
                    this.activeJoysticks.add(joystickId);
                    updatePosition(clientX);
                };

                const endDrag = () => {
                    if (!isDragging) return;
                    
                    isDragging = false;
                    this.activeJoysticks.delete(joystickId);
                    handle.style.transform = 'translate(-50%, -50%)';
                    callback(90); // Volver al centro
                };

                // Eventos táctiles
                const handleTouchStart = (e) => {
                    e.preventDefault();
                    startDrag(e.touches[0].clientX);
                };

                const handleTouchMove = (e) => {
                    e.preventDefault();
                    if (isDragging) {
                        updatePosition(e.touches[0].clientX);
                    }
                };

                const handleTouchEnd = (e) => {
                    e.preventDefault();
                    endDrag();
                };

                joystick.addEventListener('touchstart', handleTouchStart, { passive: false });
                joystick.addEventListener('touchmove', handleTouchMove, { passive: false });
                joystick.addEventListener('touchend', handleTouchEnd);
                joystick.addEventListener('touchcancel', handleTouchEnd);

                // Eventos mouse
                const handleMouseDown = (e) => {
                    startDrag(e.clientX);
                };

                const handleMouseMove = (e) => {
                    if (isDragging) {
                        updatePosition(e.clientX);
                    }
                };

                const handleMouseUp = () => {
                    endDrag();
                };

                joystick.addEventListener('mousedown', handleMouseDown);
                document.addEventListener('mousemove', handleMouseMove);
                document.addEventListener('mouseup', handleMouseUp);

                // Prevenir comportamiento por defecto
                joystick.addEventListener('dragstart', (e) => e.preventDefault());
                joystick.addEventListener('contextmenu', (e) => e.preventDefault());
            }

            sendCommand(command) {
                if (this.ws && this.ws.readyState === WebSocket.OPEN) {
                    this.ws.send(command);
                    console.log('Comando enviado:', command);
                }
            }

            setGiroStatus(activo) {
                this.giroActivo = activo;
                const statusElement = document.getElementById('giroStatus');
                const btnGiro = document.getElementById('btnGiro');
                const btnCancelar = document.getElementById('btnCancelar');
                
                if (activo) {
                    statusElement.textContent = 'Giro: ACTIVO - En progreso...';
                    statusElement.className = 'giro-status giro-active';
                    btnGiro.disabled = true;
                    btnCancelar.disabled = false;
                    this.showToast('🔄 Giro iniciado', 'warning');
                    
                    // Limpiar joysticks activos durante el giro
                    this.activeJoysticks.clear();
                    document.querySelectorAll('.joystick-handle').forEach(handle => {
                        handle.style.transform = 'translate(-50%, -50%)';
                    });
                } else {
                    statusElement.textContent = 'Giro: INACTIVO';
                    statusElement.className = 'giro-status giro-inactive';
                    btnGiro.disabled = false;
                    btnCancelar.disabled = true;
                }
            }

            iniciarGiro() {
                if (this.giroActivo) {
                    this.showToast('⏳ Ya hay un giro en progreso', 'warning');
                    return;
                }
                this.sendCommand('GIRO:INICIAR');
                this.setGiroStatus(true);
            }

            cancelarGiro() {
                if (!this.giroActivo) {
                    this.showToast('ℹ️ No hay giro activo', 'info');
                    return;
                }
                this.sendCommand('GIRO:CANCELAR');
                this.setGiroStatus(false);
                this.showToast('⏹️ Giro cancelado', 'error');
            }

            centerAllServos() {
                if (this.giroActivo) {
                    this.showToast('⏳ No se puede centrar durante giro activo', 'warning');
                    return;
                }
                this.sendCommand('X:90');
                this.sendCommand('Y:90');
                this.sendCommand('A:90');
                this.sendCommand('B:90');
                
                // Actualizar displays
                document.getElementById('servoX').textContent = '90°';
                document.getElementById('servoY').textContent = '90°';
                document.getElementById('servoA').textContent = '90°';
                document.getElementById('servoB').textContent = '90°';
                
                // Resetear joysticks visualmente
                document.querySelectorAll('.joystick-handle').forEach(handle => {
                    handle.style.transform = 'translate(-50%, -50%)';
                });
                
                this.showToast('🔄 Todos los servos centrados', 'success');
            }

            updateConnectionStatus(connected) {
                const dot = document.getElementById('connectionDot');
                const status = document.getElementById('connectionStatus');
                
                if (connected) {
                    dot.className = 'status-dot';
                    status.textContent = 'Conectado al Drone';
                } else {
                    dot.className = 'status-dot offline';
                    status.textContent = 'Desconectado';
                }
            }

            showToast(message, type = 'info') {
                const toast = document.getElementById('toast');
                toast.textContent = message;
                toast.className = `toast ${type} show`;
                
                setTimeout(() => {
                    toast.className = 'toast';
                }, 3000);
            }
        }

        // Funciones globales para los botones
        function iniciarGiro() {
            window.droneController.iniciarGiro();
        }

        function cancelarGiro() {
            window.droneController.cancelarGiro();
        }

        function centerAllServos() {
            window.droneController.centerAllServos();
        }

        // Inicializar el controlador cuando la página cargue
        document.addEventListener('DOMContentLoaded', () => {
            window.droneController = new DroneController();
        });
    </script>
</body>
</html>
)rawliteral";

void handleWebSocketMessage(uint8_t num, uint8_t *data, size_t len) {
  data[len] = 0;
  String msg = String((char*)data);

  // Comando para iniciar giro
  if (msg == "GIRO:INICIAR") {
    if (!giroActivo) {
      giroActivo = true;
      tiempoInicioGiro = millis();
      
      // Mover servos a posición de giro
      servoX.write(anguloGiro);
      servoY.write(anguloGiro);
      servoA.write(anguloGiro);
      servoB.write(anguloGiro);
      
      Serial.println("🔄 Giro único INICIADO");
      webSocket.sendTXT(num, "GIRO:INICIADO");
    }
    return;
  }
  
  // Comando para cancelar giro
  if (msg == "GIRO:CANCELAR") {
    if (giroActivo) {
      giroActivo = false;
      
      // Volver a posición central inmediatamente
      servoX.write(90);
      servoY.write(90);
      servoA.write(90);
      servoB.write(90);
      
      Serial.println("⏹️ Giro CANCELADO");
      webSocket.sendTXT(num, "GIRO:CANCELADO");
    }
    return;
  }

  // Control individual de cada servo (solo si no hay giro activo)
  if (!giroActivo) {
    // Servo X - Joystick Izquierdo Arriba/Abajo
    if (msg.startsWith("X:")) {
      int posX = 90;
      sscanf(msg.c_str(), "X:%d", &posX);
      posX = constrain(posX, 60, 120);
      servoX.write(posX);
      Serial.printf("🎮 Servo X (Arriba/Abajo): %d°\n", posX);
    }

    // Servo Y - Joystick Izquierdo Izquierda/Derecha
    if (msg.startsWith("Y:")) {
      int posY = 90;
      sscanf(msg.c_str(), "Y:%d", &posY);
      posY = constrain(posY, 60, 120);
      servoY.write(posY);
      Serial.printf("🎮 Servo Y (Izquierda/Derecha): %d°\n", posY);
    }

    // Servo A - Joystick Derecho Arriba/Abajo
    if (msg.startsWith("A:")) {
      int posA = 90;
      sscanf(msg.c_str(), "A:%d", &posA);
      posA = constrain(posA, 60, 120);
      servoA.write(posA);
      Serial.printf("🎮 Servo A (Arriba/Abajo): %d°\n", posA);
    }

    // Servo B - Joystick Derecho Izquierda/Derecha
    if (msg.startsWith("B:")) {
      int posB = 90;
      sscanf(msg.c_str(), "B:%d", &posB);
      posB = constrain(posB, 60, 120);
      servoB.write(posB);
      Serial.printf("🎮 Servo B (Izquierda/Derecha): %d°\n", posB);
    }
  }
}

void onEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("🟢 Cliente conectado");
      webSocket.sendTXT(num, "MODO:MANUAL");
      if (giroActivo) {
        webSocket.sendTXT(num, "GIRO:INICIADO");
      } else {
        webSocket.sendTXT(num, "GIRO:INACTIVO");
      }
      break;
    case WStype_DISCONNECTED:
      Serial.println("🔴 Cliente desconectado");
      break;
    case WStype_TEXT:
      handleWebSocketMessage(num, payload, length);
      break;
  }
}

// Servir la página web
void handleRoot() {
  server.send(200, "text/html", webpage);
}

void controlarGiro() {
  if (giroActivo) {
    unsigned long tiempoActual = millis();
    
    // Verificar si ha pasado el tiempo del giro
    if (tiempoActual - tiempoInicioGiro >= duracionGiro) {
      giroActivo = false;
      
      // Volver a posición central
      servoX.write(90);
      servoY.write(90);
      servoA.write(90);
      servoB.write(90);
      
      Serial.println("✅ Giro único COMPLETADO");
      
      // Notificar a todos los clientes conectados
      webSocket.broadcastTXT("GIRO:COMPLETADO");
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Configurar como Access Point (AP)
  Serial.println("\n🚀 Iniciando Access Point para Drone...");
  Serial.print("📡 Creando red: ");
  Serial.println(ssid_ap);
  
  // Crear el Access Point
  WiFi.softAP(ssid_ap, password_ap);
  
  // Esperar a que el AP esté listo
  delay(1000);
  
  // Mostrar información del AP
  Serial.println("✅ Access Point creado exitosamente");
  Serial.print("📶 Nombre de la red (SSID): ");
  Serial.println(ssid_ap);
  Serial.print("🔑 Contraseña: ");
  Serial.println(password_ap);
  Serial.print("📍 IP del Access Point: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("👉 Conecta tu dispositivo a esta red WiFi");

  // Inicializar servidor HTTP
  server.on("/", handleRoot);
  server.begin();
  Serial.println("✅ Servidor HTTP iniciado");

  // Inicializar WebSocket
  webSocket.begin();
  webSocket.onEvent(onEvent);
  Serial.println("✅ WebSocket iniciado en puerto 81");

  // Adjuntar servos
  servoX.attach(pinServoX);
  servoY.attach(pinServoY);
  servoA.attach(pinServoA);
  servoB.attach(pinServoB);

  // Posición central inicial
  servoX.write(90);
  servoY.write(90);
  servoA.write(90);
  servoB.write(90);
  
  Serial.println("🎯 Servos en posición central (90°)");
  Serial.println("🔄 Sistema de 4 joysticks independientes configurado");
  Serial.println("🚀 Drone listo - Esperando conexiones...");
  Serial.println("👉 Abre tu navegador y ve a: http://" + WiFi.softAPIP().toString());
  Serial.println("==========================================");     
}

void loop() {
  server.handleClient();  // Manejar peticiones HTTP
  webSocket.loop();       // Manejar WebSockets
  
  // Controlar el giro único si está activo
  controlarGiro();
}
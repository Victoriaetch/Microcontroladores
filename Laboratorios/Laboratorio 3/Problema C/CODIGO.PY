import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# --- Configuración ---
SERIAL_PORT = 'COM1'
BAUD_RATE = 9600
MAX_DATA_POINTS = 100  # Número de puntos a mostrar en la gráfica

# --- Listas de datos (usando deque para eficiencia) ---
data_ref = deque(maxlen=MAX_DATA_POINTS)
data_actual = deque(maxlen=MAX_DATA_POINTS)
data_pwm = deque(maxlen=MAX_DATA_POINTS)
time_axis = deque(maxlen=MAX_DATA_POINTS)  # Eje X
time_current = 0

# --- Configuración de la Gráfica ---
fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)
fig.suptitle('Control de Motor (ATmega328P)')

# Subplot 1: Posición
ax1.set_ylabel('Posición (0-1023)')
line_ref, = ax1.plot([], [], 'r-', label='Referencia')
line_actual, = ax1.plot([], [], 'b-', label='Actual (Motor)')
ax1.legend()
ax1.grid()
ax1.set_ylim(-50, 1073)  # 0-1023 con margen

# Subplot 2: PWM
ax2.set_ylabel('PWM (0-255)')
ax2.set_xlabel('Muestras')
line_pwm, = ax2.plot([], [], 'g-', label='PWM Aplicado')
ax2.legend()
ax2.grid()
ax2.set_ylim(-265, 265)  # 0-255 con margen

# --- Conexión Serial ---
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    ser.flushInput()
except Exception as e:
    print(f"Error abriendo el puerto serial {SERIAL_PORT}: {e}")
    exit()

# --- Función de Inicialización de la Animación ---
def init():
    line_ref.set_data([], [])
    line_actual.set_data([], [])
    line_pwm.set_data([], [])
    return line_ref, line_actual, line_pwm,

# --- Función de Actualización (Animación) ---
def update(frame):
    global time_current
    try:
        if ser.in_waiting > 0:
            # Leer y decodificar la línea
            line = ser.readline().decode('utf-8').strip()
            print(f"Dato recibido: [{line}]")
            # Parsear los datos (CSV: Ref,Actual,PWM,Dir)
            parts = line.split(',')
            if len(parts) == 4:
                ref = int(parts[0])
                actual = int(parts[1])
                pwm_val = int(parts[2])
                direction = int(parts[3])

                # Si la dirección es antihoraria, graficamos el PWM como negativo
                if direction == -1:
                    pwm_val = -pwm_val

                # Añadir datos a las listas
                data_ref.append(ref)
                data_actual.append(actual)
                data_pwm.append(pwm_val)
                time_axis.append(time_current)
                time_current += 1

                # Actualizar las líneas de la gráfica
                line_ref.set_data(time_axis, data_ref)
                line_actual.set_data(time_axis, data_actual)
                line_pwm.set_data(time_axis, data_pwm)

                # Auto-ajustar eje X
                ax1.set_xlim(min(time_axis), max(time_axis) + 1)

    except Exception as e:
        print(f"Error leyendo datos: {e}")
        # Intenta ignorar líneas malformadas
        pass

    return line_ref, line_actual, line_pwm,

# --- Iniciar Animación ---
ani = animation.FuncAnimation(fig, update, init_func=init, blit=True, interval=20)
plt.show()

# Cerrar puerto serial al salir
ser.close()
print("Puerto serial cerrado.")

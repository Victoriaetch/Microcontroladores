import serial
import serial.tools.list_ports
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import TextBox  

PUERTO_COM = 'COM1'
BAUDRATE = 9600
MAX_POINTS = 100
INTERVALO_MS = 1000 

temps = []
heater = []
fan = []
pm_values = []

try:
    ser = serial.Serial(PUERTO_COM, BAUDRATE, timeout=1)
    ser.flushInput()
    print(f"Conectado a {PUERTO_COM} a {BAUDRATE} baud.")
except serial.SerialException as e:
    print(f"Error: No se pudo abrir el puerto {PUERTO_COM}.")
    exit()

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 7), sharex=True)

plt.subplots_adjust(bottom=0.2)
line_temp, = ax1.plot([], [], label="Temperatura (°C)", color="tab:red", linewidth=2)
line_pm, = ax1.plot([], [], "--", label="Punto Medio (°C)", color="tab:blue", linewidth=1.5)
ax1.set_ylabel("Temperatura (°C)")
ax1.set_title("Evolución de la temperatura y punto medio (Tiempo Real)")
ax1.legend(loc="upper right")
ax1.grid(True)

line_heater, = ax2.plot([], [], label="Calefactor (0=OFF, 1–2=ON)", color="tab:orange", linewidth=2)
line_fan, = ax2.plot([], [], label="Ventilador (0–3)", color="tab:green", linewidth=2)
ax2.set_xlabel(f"Últimas {MAX_POINTS} muestras")
ax2.set_ylabel("Estado")
ax2.legend(loc="upper right")
ax2.grid(True)

ax1.set_xlim(0, MAX_POINTS)
ax1.set_ylim(-5, 155)
ax2.set_ylim(-0.5, 3.5)

def update(frame):
    if not ser.in_waiting > 0:
        return line_temp, line_pm, line_heater, line_fan

    try:
        line = ser.readline().decode('utf-8').strip()
        if not line:
            return line_temp, line_pm, line_heater, line_fan

        if line.count(',') != 3:
            print(f"Recibido: {line}") 
            return line_temp, line_pm, line_heater, line_fan

        t, h, f_, pm = line.split(",")

        temps.append(float(t))
        heater.append(int(h))
        fan.append(int(f_))
        pm_values.append(float(pm))

        del temps[:-MAX_POINTS]
        del heater[:-MAX_POINTS]
        del fan[:-MAX_POINTS]
        del pm_values[:-MAX_POINTS]

    except Exception as e:
        print(f"Error parseando línea: '{line}'. Error: {e}")
        return line_temp, line_pm, line_heater, line_fan

    x_data = range(len(temps))
    line_temp.set_data(x_data, temps)
    line_pm.set_data(x_data, pm_values)
    line_heater.set_data(x_data, heater)
    line_fan.set_data(x_data, fan)

    ax1.relim()
    ax1.autoscale_view(scalex=False)
    ax2.relim()
    ax2.autoscale_view(scalex=False)

    return line_temp, line_pm, line_heater, line_fan

def submit_pm(text):
    try:
        nuevo_pm = int(text)
        if 0 <= nuevo_pm <= 99:
            print(f"Enviando nuevo PM: {nuevo_pm}")
            ser.write(f"{nuevo_pm}\n".encode('utf-8'))
        else:
            print("Valor de PM fuera de rango (0-99)")
    except ValueError:
        print(f"Valor inválido: '{text}' no es un número entero.")
    except Exception as e:
        print(f"Error enviando datos: {e}")

ax_box = plt.axes([0.3, 0.05, 0.4, 0.075])

text_box = TextBox(ax_box, 'Nuevo PM (Enter):', initial="")
text_box.on_submit(submit_pm)
fig.text_box = text_box

ani = FuncAnimation(fig, update, interval=INTERVALO_MS, blit=True, cache_frame_data=False)

try:
    plt.show()
except Exception as e:
    print(f"Se cerró la ventana de gráfico. {e}")

finally:
    ser.close()
    print("Puerto serial cerrado. Saliendo.")

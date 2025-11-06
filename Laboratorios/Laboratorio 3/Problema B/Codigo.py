import matplotlib.pyplot as plt

temps = []
heater = []
fan = []
pm_values = []

with open("datos.txt", "r") as f:
    for line in f:
        try:
            t, h, f_, pm = line.strip().split(",")
            temps.append(float(t))
            heater.append(int(h))
            fan.append(int(f_))
            pm_values.append(float(pm))
        except:
            pass

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 7), sharex=True)

ax1.plot(temps, label="Temperatura (°C)", color="tab:red", linewidth=2)
ax1.plot(pm_values, "--", label="Punto Medio (°C)", color="tab:blue", linewidth=1.5)

ax1.set_ylabel("Temperatura (°C)")
ax1.set_title("Evolución de la temperatura y punto medio")
ax1.legend(loc="upper right")
ax1.grid(True)

ax2.plot(heater, label="Calefactor (0=OFF, 1–2=ON)", color="tab:orange", linewidth=2)
ax2.plot(fan, label="Ventilador (0–3)", color="tab:green", linewidth=2)

ax2.set_xlabel("Muestras (cada 1s)")
ax2.set_ylabel("Estado")
ax2.set_title("Estados del calefactor y ventilador")
ax2.legend(loc="upper right")
ax2.grid(True)

plt.tight_layout()
plt.show()

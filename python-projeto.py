import serial
import serial.tools.list_ports
from obsws_python import ReqClient
import time

# ================= OBS =================
client = ReqClient(
    host="localhost",
    port=4455,
    password=""
)

MIC_SOURCE = "FIFINE mic"
DESKTOP_SOURCE = "Xbox audio"

print("✔ Conectado ao OBS")

# ================= ESP32 =================
ser = None
print("Procurando ESP32...")

for porta in serial.tools.list_ports.comports():
    try:
        print(f"Tentando {porta.device}")

        ser = serial.Serial(porta.device, 115200, timeout=1)
        time.sleep(2)

        print(f"✔ ESP32 conectado em {porta.device}")
        break

    except:
        pass

if ser is None:
    print("❌ ESP32 não encontrado")
    quit()

mic_raw = 0
cena_raw = 0

mic_smooth = 0
cena_smooth = 0

def set_volume(source, value):
    try:
        value = max(0, min(value, 100))
        client.set_input_volume(source, value / 100.0)
    except Exception as e:
        print(f"❌ Erro OBS ({source}):", e)

while True:
    try:
        linha = ser.readline().decode("utf-8", errors="ignore").strip()

        if not linha:
            continue

        print("RAW:", linha)

        # ===== PARSE CORRETO =====
        if "MIC:" in linha and "CENA:" in linha:
            try:
                partes = linha.split()

                for p in partes:
                    if p.startswith("MIC:"):
                        mic_raw = int(p.split(":")[1])

                    elif p.startswith("CENA:"):
                        cena_raw = int(p.split(":")[1])

            except:
                continue

        # suavização estilo mixer
        mic_smooth = mic_smooth * 0.85 + mic_raw * 0.15
        cena_smooth = cena_smooth * 0.85 + cena_raw * 0.15

        set_volume(MIC_SOURCE, mic_smooth)
        set_volume(DESKTOP_SOURCE, cena_smooth)

        print(f"MIC {mic_smooth:.0f}% | CENA {cena_smooth:.0f}%")

        time.sleep(0.02)

    except Exception as e:
        print("Erro geral:", e)
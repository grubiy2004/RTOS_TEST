import sys
import serial
from serial.tools import list_ports
import numpy as np
import pyqtgraph as pg
from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import QTimer

# --- Настройки ---
BAUD = 115200
N = 256
FS = 20000
FFT_SIZE = 512
BIN_HZ = FS / FFT_SIZE
SYNC = b'\xA5\xA5\xA5\xA5'

# --- Автопоиск CP2102 ---
VID_CP2102 = 0x10C4
PID_CP2102 = 0xEA60

def find_cp2102_port():
    for port in list_ports.comports():
        if port.vid == VID_CP2102 and port.pid == PID_CP2102:
            return port.device
    return None

PORT = find_cp2102_port()
if PORT is None:
    print("CP2102 не найден. Доступные порты:")
    for p in list_ports.comports():
        vid = hex(p.vid) if p.vid else 'N/A'
        pid = hex(p.pid) if p.pid else 'N/A'
        print(f"  {p.device} — {p.description} (VID={vid}, PID={pid})")
    sys.exit(1)

print(f"Найден CP2102 на порту: {PORT}")
ser = serial.Serial(PORT, BAUD, timeout=0.1)
buffer = b''

freqs = np.arange(N) * BIN_HZ

# --- GUI ---
app = QApplication(sys.argv)
win = pg.GraphicsLayoutWidget(show=True, title=f"Спектр ({PORT})")
win.resize(1000, 500)
plot = win.addPlot()
plot.setLabel('left', 'Магнитуда')
plot.setLabel('bottom', 'Частота', units='Гц')
plot.setXRange(0, FS / 2)
plot.setYRange(0, 1000)
plot.showGrid(x=True, y=True, alpha=0.3)
curve = plot.plot(pen=pg.mkPen('y', width=1))

# --- 3 текстовые метки для пиков ---
peak_labels = []
for i in range(3):
    label = pg.TextItem(text='', color='r', anchor=(0.5, 1.2))
    label.setFont(pg.Qt.QtGui.QFont('Arial', 10, pg.Qt.QtGui.QFont.Bold))
    plot.addItem(label)
    peak_labels.append(label)

# --- Поиск пиков ---
def find_peaks(values, n_peaks=3, min_distance=5):
    values = values.copy()
    peaks = []
    for _ in range(n_peaks):
        idx = int(np.argmax(values))
        if values[idx] <= 0:
            break
        peaks.append(idx)
        lo = max(0, idx - min_distance)
        hi = min(len(values), idx + min_distance + 1)
        values[lo:hi] = 0
    return peaks

def update():
    global buffer
    chunk = ser.read(ser.in_waiting or 1)
    if chunk:
        buffer += chunk

    idx = buffer.find(SYNC)
    if idx < 0:
        if len(buffer) > 3:
            buffer = buffer[-3:]
        return

    if idx > 0:
        buffer = buffer[idx:]

    packet_size = len(SYNC) + 2 * N
    if len(buffer) < packet_size:
        return

    data = buffer[len(SYNC):len(SYNC) + 2 * N]
    values = np.frombuffer(data, dtype='<i2')
    curve.setData(freqs, values)
    buffer = buffer[packet_size:]

    # --- Поиск и подпись 3 пиков ---
    peaks = find_peaks(values.astype(float), n_peaks=3, min_distance=5)
    for i, label in enumerate(peak_labels):
        if i < len(peaks):
            bin_idx = peaks[i]
            freq = freqs[bin_idx]
            mag = values[bin_idx]
            label.setText(f"{freq:.1f} Гц")
            label.setPos(freq, mag)
        else:
            label.setText('')

timer = QTimer()
timer.timeout.connect(update)
timer.start(100)

sys.exit(app.exec_())

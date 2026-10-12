import sys
import serial
import numpy as np
import pyqtgraph as pg
from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import QTimer

# --- Настройки ---
PORT = 'COM3'
BAUD = 115200
N = 256                          # количество q15-значений
FS = 20000                       # частота дискретизации
FFT_SIZE = 512
BIN_HZ = FS / FFT_SIZE           # 39.06 Гц на бин
SYNC = b'\xA5\xA5\xA5\xA5'       # 4 байта синхронизации

freqs = np.arange(N) * BIN_HZ    # ось X: 0..10 кГц

ser = serial.Serial(PORT, BAUD, timeout=0.1)
buffer = b''

# --- GUI ---
app = QApplication(sys.argv)
win = pg.GraphicsLayoutWidget(show=True, title="Спектр")
win.resize(1000, 500)
plot = win.addPlot()
plot.setLabel('left', 'Магнитуда')
plot.setLabel('bottom', 'Частота', units='Гц')
plot.setXRange(0, FS / 2)
plot.enableAutoRange(axis='y')
plot.showGrid(x=True, y=True, alpha=0.3)
curve = plot.plot(pen=pg.mkPen('y', width=1))

# --- Поиск пакета ---
def find_sync(buf):
    """Ищет позицию SYNC в буфере. Возвращает индекс начала или -1."""
    idx = buf.find(SYNC)
    return idx

def update():
    global buffer
    # Читаем всё, что пришло
    chunk = ser.read(ser.in_waiting or 1)
    if chunk:
        buffer += chunk

    # Ищем начало пакета
    idx = find_sync(buffer)
    if idx < 0:
        # Sync не найден — оставляем хвост (последние 3 байта могут быть началом A5 A5 A5)
        if len(buffer) > 3:
            buffer = buffer[-3:]
        return

    # Отрезаем всё до sync
    if idx > 0:
        buffer = buffer[idx:]

    # Проверяем, есть ли полный пакет: SYNC + 2*N байт
    packet_size = len(SYNC) + 2 * N
    if len(buffer) < packet_size:
        return   # ждём ещё

    # Берём данные ПОСЛЕ sync
    data_start = len(SYNC)
    data = buffer[data_start:data_start + 2 * N]
    values = np.frombuffer(data, dtype='<i2')

    # Обновляем график
    curve.setData(freqs, values)

    # Удаляем использованный пакет из буфера
    buffer = buffer[packet_size:]

    # Ограничиваем буфер, чтобы не рос бесконечно
    if len(buffer) > 4 * packet_size:
        buffer = buffer[-2 * packet_size:]

timer = QTimer()
timer.timeout.connect(update)
timer.start(100)

sys.exit(app.exec_())

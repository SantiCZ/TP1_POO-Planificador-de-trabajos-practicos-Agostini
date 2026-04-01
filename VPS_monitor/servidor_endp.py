#!/usr/bin/env python3
"""
VPS Health Endpoint — Ejercicio 02
====================================
Endpoint mínimo que devuelve métricas de salud del servidor en JSON.
Instalar dependencias:
    pip install flask psutil

Ejecutar:
    python3 vps_health_endpoint.py

El panel Qt debe apuntar a:
    http://localhost:5000/api/health
    (o http://<tu-ip-publica>:5000/api/health si está en un VPS real)

Para exponer en producción se recomienda usar gunicorn + nginx:
    gunicorn -w 2 -b 0.0.0.0:5000 vps_health_endpoint:app
"""

from flask import Flask, jsonify
import psutil
import time
import datetime

app = Flask(__name__)

# Referencia de arranque del proceso para calcular uptime
_start_time = time.time()


def _format_uptime(seconds: float) -> str:
    """Convierte segundos en string legible: '3d 14h 22m'"""
    d = int(seconds // 86400)
    h = int((seconds % 86400) // 3600)
    m = int((seconds % 3600) // 60)
    parts = []
    if d: parts.append(f"{d}d")
    if h: parts.append(f"{h}h")
    parts.append(f"{m}m")
    return " ".join(parts) if parts else "0m"


def _get_network_kbps() -> tuple[float, float]:
    """
    Mide el tráfico de red durante 0.5 s y devuelve (rx_kbps, tx_kbps).
    En un VPS real se puede usar el delta respecto al chequeo anterior
    para mayor precisión; aquí se usa una ventana corta para simplicidad.
    """
    before = psutil.net_io_counters()
    time.sleep(0.5)
    after = psutil.net_io_counters()
    rx_kbps = (after.bytes_recv - before.bytes_recv) / 1024 / 0.5
    tx_kbps = (after.bytes_sent - before.bytes_sent) / 1024 / 0.5
    return round(rx_kbps, 2), round(tx_kbps, 2)


@app.route("/api/health", methods=["GET"])
def health():
    """
    Devuelve el estado de salud del servidor en JSON.

    Formato de respuesta:
    {
        "cpu_load":       34.5,
        "mem_used_pct":   62.1,
        "disk_used_pct":  47.8,
        "network_rx_kbps": 120.4,
        "network_tx_kbps":  55.2,
        "uptime":         "3d 14h 22m",
        "status":         "ok",
        "timestamp":      "2025-03-30T14:22:05"
    }
    """
    cpu    = psutil.cpu_percent(interval=0.2)
    mem    = psutil.virtual_memory().percent
    disk   = psutil.disk_usage("/").percent
    rx_kbps, tx_kbps = _get_network_kbps()

    uptime_secs = time.time() - _start_time   # uptime del proceso
    # Para uptime real del SO se puede usar: uptime_secs = time.time() - psutil.boot_time()
    uptime_secs_os = time.time() - psutil.boot_time()

    # Clasificación simple del estado
    if cpu >= 95 or mem >= 95:
        status = "critical"
    elif cpu >= 80 or mem >= 85:
        status = "warning"
    else:
        status = "ok"

    payload = {
        "cpu_load":         round(cpu, 2),
        "mem_used_pct":     round(mem, 2),
        "disk_used_pct":    round(disk, 2),
        "network_rx_kbps":  rx_kbps,
        "network_tx_kbps":  tx_kbps,
        "uptime":           _format_uptime(uptime_secs_os),
        "status":           status,
        "timestamp":        datetime.datetime.now().isoformat(timespec="seconds"),
    }

    response = jsonify(payload)
    # Permitir peticiones desde cualquier origen (necesario si el panel
    # corre en la misma máquina que el servidor).
    response.headers.add("Access-Control-Allow-Origin", "*")
    return response


@app.route("/", methods=["GET"])
def index():
    return (
        "<h2>VPS Health Endpoint — Ejercicio 02</h2>"
        "<p>Endpoint disponible en <a href='/api/health'>/api/health</a></p>"
    )


if __name__ == "__main__":
    # debug=False en producción
    app.run(host="0.0.0.0", port=5003, debug=False)

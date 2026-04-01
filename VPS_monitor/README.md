# VPS Monitor — Ejercicio 02
Panel de monitoreo de servidor en la nube, desarrollado con Qt (C++17).

---

## Arquitectura del proyecto

```
vps_monitor/
├── vps_monitor.pro          ← Proyecto qmake (empty qmake project)
├── vps_health_endpoint.py   ← Endpoint Flask para el VPS
├── resources/
│   └── resources.qrc
└── src/
    ├── main.cpp             ← Punto de entrada
    ├── mainwindow.h/.cpp    ← Ventana principal + UI
    ├── servermonitor.h/.cpp ← Clase de lógica de monitoreo (QObject)
    ├── metriccardwidget.h/.cpp  ← Tarjeta de métrica con sparkline
    ├── sparklinewidget.h/.cpp   ← Mini-gráfica de historial
    ├── statusbadge.h/.cpp       ← Pastilla de estado animada
    └── eventlogwidget.h/.cpp    ← Historial de eventos
```

---

## Clases propias

| Clase | Responsabilidad |
|---|---|
| `ServerMonitor` | Lógica de polling HTTP, parseo JSON, evaluación de umbrales, historial |
| `MetricCardWidget` | Tarjeta visual de métrica: anillo o barra + sparkline |
| `SparklineWidget` | Mini-gráfica de línea con relleno degradado |
| `StatusBadge` | Pastilla de estado con punto pulsante animado |
| `EventLogWidget` | Lista de eventos con colores por severidad |

`ServerMonitor` contiene **toda** la lógica de negocio: polling, parseo, clasificación de estado, historial. `MainWindow` solo coordina la UI.

---

## Widgets interactivos y su justificación

| Widget | Rol | Justificación |
|---|---|---|
| `QLineEdit` | URL del endpoint | Texto libre: la URL varía por usuario/servidor |
| `QSpinBox` | Intervalo de chequeo | Valor entero acotado (5–3600 s); fácil de incrementar/decrementar |
| `QDoubleSpinBox` | Umbral CPU % / RAM % | Valor decimal fino; permite ajuste preciso ej. 82.5 % |
| `QPushButton` (▶ Iniciar / ■ Detener) | Control del ciclo de polling | Acción binaria principal, visualmente clara |
| `QPushButton` (⟳ Ahora) | Refresco manual | Permite chequeo inmediato sin esperar el intervalo |
| `QPushButton` (Limpiar) | Vaciar log de eventos | Acción destructiva secundaria, tamaño pequeño intencional |
| `QLabel` | Último chequeo, uptime, countdown | Lectura rápida de datos no editables |
| `StatusBadge` (custom) | Estado general OK/ALERTA/CRÍTICO/CAÍDO | Visual de un vistazo, inspirado en Grafana |
| `MetricCardWidget` (custom) | CPU, RAM, Disco, Red RX/TX | Tarjeta compacta con valor + historial |

---

## Endpoint JSON esperado

```json
{
  "cpu_load":         34.5,
  "mem_used_pct":     62.1,
  "disk_used_pct":    47.8,
  "network_rx_kbps": 120.4,
  "network_tx_kbps":  55.2,
  "uptime":          "3d 14h 22m",
  "status":          "ok"
}
```

El campo `status` es opcional; si se omite, la app lo calcula con los umbrales configurados.

---

## Compilar y ejecutar

### Requisitos
- Qt 5.15+ o Qt 6.x (módulos: `core gui widgets network`)
- Compilador C++17 (GCC, Clang o MSVC)

### Desde Qt Creator
1. Abrir `vps_monitor.pro` como **Empty qmake Project**
2. Configurar el kit deseado
3. Build → Run

### Desde línea de comandos
```bash
cd vps_monitor
qmake vps_monitor.pro
make -j$(nproc)
./vps_monitor
```

---

## Levantar el endpoint en el VPS

```bash
# Instalar dependencias
pip install flask psutil

# Ejecutar localmente para pruebas
python3 vps_health_endpoint.py

# El panel debe apuntar a:
# http://localhost:5000/api/health

# En producción (VPS real con IP pública):
# http://<tu-ip>:5000/api/health
# Se recomienda nginx como proxy reverso + HTTPS
```

---

## Inspiración de diseño

- **Grafana** — tarjetas de métricas, sparklines, tema oscuro
- **Netdata** — anillos de CPU/RAM, historial corto, colores semáforo
- **Datadog** — statusbadge con dot pulsante, log de eventos con severidad
- **Prometheus** — separación clara de lógica (scraper) y presentación (UI)

---

## Flujo de datos

```
QTimer (intervalSpin s)
    │
    ▼
ServerMonitor::checkNow()
    │  QNetworkAccessManager::get()
    ▼
HTTP GET /api/health
    │  JSON response
    ▼
ServerMonitor::parseReply()
    ├── evaluateThresholds()   → status: ok/warning/critical
    ├── m_cpuHistory.append()  → sparkline data
    └── emit metricsUpdated()
            │
            ▼
    MainWindow::onMetricsUpdated()
        ├── StatusBadge::setStatus()
        ├── MetricCardWidget::setValue()
        ├── MetricCardWidget::setHistory()
        └── QLabel (uptime, timestamp)
```
## aclaracion 
para usar el monitoreo se debe introducir la siguiente direccion en la aplicacion qt: http://161.97.92.143:5005/api/health 
# Lienzo Colaborativo en Tiempo Real
### Ejercicio 03 — Programacion Orientada a Objetos

Aplicacion de escritorio desarrollada en **Qt/C++** que permite dibujar a mano alzada sobre un lienzo compartido, con sincronizacion colaborativa en tiempo real a traves de un servidor VPS.

---

## Caracteristicas

- Dibujo libre a mano alzada con suavizado visual mediante interpolacion **Catmull-Rom**
- Colaboracion en tiempo real entre multiples usuarios
- Sincronizacion automatica al terminar cada trazo
- Fusion incremental de trazos (nunca se pierde el trabajo de ningun usuario)
- Paleta de 9 colores interpolados
- Control dinamico del grosor del trazo
- Goma de borrar
- Boton de guardado manual con estilo Metro de Windows

---

## Arquitectura

El proyecto sigue el patron **Modelo–Vista–Controlador** con tres clases principales:

### `DrawingModel`
Responsable del almacenamiento y logica de los trazos.
- Almacena todos los trazos como `QVector<Stroke>`
- Cada trazo tiene un **UUID inico**, color, grosor, timestamp y lista de puntos
- Implementa interpolacion **Catmull-Rom** propia para suavizado
- `mergeStrokes()` realiza fusion incremental por UUID: solo agrega trazos nuevos, nunca sobreescribe

### `CanvasView`
Responsable del renderizado, hereda de `QWidget`.
- Renderiza con `paintEvent` usando **doble capa**:
  - `m_canvasImage` → trazos terminados (persistente)
  - `m_overlayImage` → trazo en curso (actualizacion fluida)
- Interpolacion adaptativa: si el mouse se mueve rapido, agrega puntos intermedios automaticamente
- Click izquierdo = lapiz, click derecho = goma
- Rueda del mouse = control de grosor
- Teclas 1–9 = seleccion de color

### `SyncManager`
Responsable de toda la comunicacion con el VPS.
- Usa `QNetworkAccessManager` para llamadas HTTP asincronas
- `saveToServer()` → POST con todos los trazos locales
- `fetchFromServer()` → GET + merge automatico
- Poll automatico cada **2 segundos**

---

## Estructura del proyecto

```
collaborative_canvas/
├── collaborative_canvas.pro   — archivo de proyecto Qt
├── main.cpp                   — punto de entrada
├── DrawingModel.h / .cpp      — modelo: almacenamiento de trazos
├── CanvasView.h / .cpp        — vista: renderizado con paintEvent
├── SyncManager.h / .cpp       — sincronizacion con VPS
├── MainWindow.h / .cpp        — ventana principal y toolbar
└── server/
    ├── server.js              — backend Node.js/Express
    └── package.json
```

---

## Compilacion

### Requisitos
- Qt 5.15+ o Qt 6.x
- Modulos Qt: `core gui widgets network`
- Compilador con soporte C++17

### Pasos
```bash
cd collaborative_canvas/
qmake collaborative_canvas.pro
make
```

En Windows con Qt Creator: abrir el `.pro` y presionar **Build**.

> **Importante:** no abrir el proyecto desde una carpeta de OneDrive. Copiar a una ruta local como `C:\Proyectos\TP3_POO\`.

---

## Servidor VPS

### Requisitos
- Node.js 16+
- npm

### Instalacion y ejecucion
```bash
cd collaborative_canvas/server/
npm install express cors
node server.js
```

Para que quede corriendo permanentemente con PM2:
```bash
npm install -g pm2
PORT=3001 pm2 start server.js --name canvas
pm2 save
pm2 startup
```

### Endpoints disponibles

| Metodo | Ruta | Descripcion |
|--------|------|-------------|
| `GET` | `/drawing` | Recupera todos los trazos |
| `POST` | `/drawing` | Merge e guarda trazos nuevos |
| `DELETE` | `/drawing` | Limpia el canvas del servidor |
| `GET` | `/health` | Estado del servidor |

### Verificar que funciona
```bash
curl http://localhost:3001/drawing
# Respuesta esperada: {"strokes":[]}
```

---

## Uso de la aplicacion

### Primera vez
Al iniciar, la app pide la URL del servidor VPS:
```
http://161.97.92.143:3001
```
Esta URL se guarda automaticamente para la proxima vez.

### Controles

| Accion | Control |
|--------|---------|
| Dibujar | Click izquierdo + arrastrar |
| Borrar | Click derecho + arrastrar |
| Cambiar color | Teclas **1** al **9** |
| Cambiar grosor | Rueda del mouse (scroll) |
| Guardar manualmente | Boton **GUARDAR** |

### Paleta de colores

Las teclas 1–9 seleccionan colores con interpolacion lineal entre:

| Tecla | Color |
|-------|-------|
| `1` | RGB(192, 19, 76) — Rosa/Rojo |
| `5` | RGB(108, 126, 137) — Intermedio |
| `9` | RGB(24, 233, 199) — Cyan/Verde |

---

## Colaboracion en tiempo real

El sistema funciona de la siguiente manera:

1. Al terminar cada trazo, se **sube automaticamente** al servidor
2. Cada 2 segundos, la app **descarga** los trazos nuevos de otros usuarios
3. El merge es **incremental por UUID**: cada trazo tiene un ID unico, y solo se agregan los que no existen localmente
4. **Nunca se pierde informacion**: ni los trazos propios ni los de otros usuarios

```
Usuario A dibuja → trazo sube al servidor
Usuario B (2 seg despues) → descarga trazo de A → aparece en su canvas
Usuario B dibuja → trazo sube al servidor
Usuario A (2 seg despues) → descarga trazo de B → aparece en su canvas
```

---

## Modelo de datos

Cada trazo se serializa como JSON:

```json
{
  "strokes": [
    {
      "id": "a3f2c1d4-...",
      "color": "#c0134c",
      "thickness": 8,
      "isEraser": false,
      "timestamp": 1712000000000,
      "points": [
        { "x": 100.0, "y": 150.0 },
        { "x": 112.0, "y": 163.0 }
      ]
    }
  ]
}
```

---

## Estrategia de merge (sin perdida de datos)

Tanto el cliente como el servidor aplican la misma logica:

1. Se construye un conjunto de IDs conocidos
2. Solo se agregan los trazos cuyo ID **no existe** en ese conjunto
3. Los trazos se ordenan por `timestamp` para preservar el orden visual
4. Resultado: el canvas final es la **union** de todos los trazos de todos los usuarios

---

## Integrantes

- Santiago Agostini
- Lorenzo Poletto 
- Avril Ogas 

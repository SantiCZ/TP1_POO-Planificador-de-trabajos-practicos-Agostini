# TP1_POO-Planificador-de-trabajos-practicos-Agostini
# Planificador de Trabajos Practicos

Aplicacion de escritorio desarrollada en **C++ con Qt6** para gestionar y hacer seguimiento de trabajos practicos universitarios. Permite registrar TPs, controlar su estado y prioridad, agregar notas y mantener un historial de acciones, todo con persistencia local en archivos JSON.

---

## Caracteristicas

- **Login con usuarios locales** - autenticacion contra un archivo `data/usuarios.json`. Incluye usuarios de prueba (`admin/admin`, `alumno/1234`) y permite registrar nuevos usuarios.
- **Sesion persistente de 5 minutos** — la sesion se guarda en disco. Si cerras y reabris la app dentro del tiempo limite, no te vuelve a pedir login. Al expirar, cierra la sesion automaticamente.
- **Tablero de TPs con grilla** — lista todos tus trabajos practicos con badges de estado y prioridad. Se actualiza en tiempo real al filtrar.
- **Filtros** — filtra por estado (Pendiente, En curso, Entregado, Vencido), por prioridad (Alta, Media, Baja) y por busqueda de texto libre.
- **Alta, edicion y eliminacion** de TPs con confirmacion al borrar.
- **Editor de notas** — cada TP tiene un campo de notas con guardado manual independiente del guardado del TP.
- **Historial de acciones** — cada operacion queda registrada con usuario, descripcion y fecha/hora, visible en la UI y persistida en `data/historial.json`.

---

## Requisitos

- Qt 6.x (con modulos `Core` y `Widgets`)
- CMake 3.16 o superior
- Compilador C++17 (GCC, Clang o MSVC)

---

## Compilacion

```bash
# 1. Clonar o descomprimir el proyecto
cd PlanificadorTP

# 2. Crear carpeta de build y compilar
mkdir build && cd build
cmake ..
cmake --build .

# 3. Ejecutar
./PlanificadorTP          # Linux / macOS
PlanificadorTP.exe        # Windows
```

> En Qt Creator: abri el `CMakeLists.txt` como proyecto, configura un kit de Qt6 y presiona **Run**.

---

## Estructura de archivos

```
PlanificadorTP/
├── CMakeLists.txt
│
├── main.cpp                  ← Punto de entrada. Orquesta pantallas con QStackedWidget
│
├── Usuario.h / .cpp          ← Modelo: datos de un usuario
├── TrabajoPractico.h / .cpp  ← Modelo: datos de un TP (nombre, estado, prioridad, notas, etc.)
├── AccionHistorial.h / .cpp  ← Modelo: una entrada del historial
│
├── UsuarioRepository.h / .cpp    ← Persiste usuarios en data/usuarios.json
├── TPRepository.h / .cpp         ← Persiste TPs en data/trabajos.json
├── HistorialRepository.h / .cpp  ← Persiste historial en data/historial.json
│
├── SessionManager.h / .cpp   ← Gestiona sesion con timer de 5 minutos
│
├── LoginWidget.h / .cpp      ← Pantalla de login y registro
├── MainWidget.h / .cpp       ← Pantalla principal con grilla y filtros
├── TPRowWidget.h / .cpp      ← Widget de una fila del tablero
├── TPEditorDialog.h / .cpp   ← Pantalla de alta/edicion de TP y notas
└── HistorialWidget.h / .cpp  ← Pantalla de historial de acciones
```

Los datos se guardan automaticamente en una carpeta `data/` dentro del directorio de ejecucion:

```
data/
├── usuarios.json    ← Usuarios registrados
├── trabajos.json    ← Trabajos practicos
├── historial.json   ← Historial de acciones
└── session.json     ← Sesion activa (se borra al cerrar o expirar)
```

---

## Uso paso a paso

### 1. Iniciar sesion

Al abrir la app aparece la pantalla de login. Podes usar los usuarios de prueba o crear uno nuevo con el boton **Registrarse**.

El contador en la pantalla principal muestra el tiempo restante de sesion. Cada vez que interactuas (crear, editar, filtrar) el timer **no** se reinicia — la sesion dura exactamente 5 minutos desde que iniciaste.

### 2. Crear un TP

Desde el tablero principal, presiona **+ Nuevo TP**. Completa:

| Campo | Descripcion |
|---|---|
| Nombre | Obligatorio. Ej: `Algoritmos — TP 1` |
| Descripcion | Texto breve opcional |
| Estado | Pendiente / En curso / Entregado / Vencido |
| Prioridad | Alta / Media / Baja |
| Fecha de entrega | Selector de calendario |
| Notas | Texto libre para apuntes, links, etc. |

Presiona **Guardar TP** para confirmar.

### 3. Editar un TP

En el tablero, presiona **Editar** en la fila del TP que queres modificar. Se abre el mismo editor con los datos cargados.

### 4. Guardar notas sin cerrar el editor

Dentro del editor, podes modificar las notas y presionar **Guardar notas** para persistirlas sin necesidad de guardar el TP completo. Aparece un mensaje con la hora exacta del guardado.

### 5. Filtrar TPs

Usa los tres controles de la barra de filtros:

- **Combo de estado** — muestra solo TPs con ese estado
- **Combo de prioridad** — muestra solo TPs con esa prioridad
- **Buscador de texto** — filtra por nombre o descripcion en tiempo real

Los filtros se combinan entre si.

### 6. Eliminar un TP

Presiona **Eliminar** en la fila correspondiente. La app pide confirmacion antes de borrar. La accion queda registrada en el historial.

### 7. Ver el historial

Presiona **Historial** en la barra superior. Se muestra una lista con todas las acciones del usuario actual: creaciones, ediciones, guardados de notas, eliminaciones, logins y logouts, con fecha y hora. Las entradas estan coloreadas por tipo de accion.

### 8. Cerrar sesion

Presiona **Cerrar sesion**. La sesion se borra del disco y volves al login.

---

## Decisiones de diseño

- **Sin QMainWindow** — la ventana raiz es un `QStackedWidget` (QWidget puro), cumpliendo la restriccion del enunciado.
- **Sin QML** — toda la UI usa widgets Qt clasicos (`QWidget`, `QGridLayout`, `QFormLayout`, etc.).
- **Patrón Repository** — cada entidad tiene su propia clase de persistencia, separada del modelo y de la UI. Esto facilita cambiar el formato de almacenamiento sin tocar la logica de pantallas.
- **Persistencia JSON** — se usa `QJsonDocument` de Qt, sin dependencias externas.
- **SessionManager como QObject** — emite señales (`tickSecond`, `sessionExpired`) que la UI conecta directamente, manteniendo la logica de sesion desacoplada de las pantallas.

---

## Usuarios de prueba

| Usuario | Contraseña |
|---|---|
| admin | admin |
| alumno | 1234 |

Podes crear nuevos usuarios desde la pantalla de login. Se guardan inmediatamente en `data/usuarios.json`.

# 📋 Kanban: Sistema Colaborativo de Gestión de Tareas

## 🏫 Contexto Académico
* **Materia**: Programación Orientada a Objetos (POO)
* **Proyecto**: Ejercicio 04 - Desarrollo de un Sistema Kanban Multiusuario
* **Institución**: [Nombre de tu Facultad]
* **Año**: 2026

---

## 👥 Integrantes del Grupo
* **Poletto Lorenzo**
* **Ogas Avril**
* **Agostini Santiago**

---

## 🚀 Descripción del Proyecto
Este sistema es una plataforma **Kanban colaborativa** que permite la gestión de flujos de trabajo en tiempo real. Utiliza una arquitectura distribuida donde un servidor centralizado (VPS) orquestado por Docker gestiona la persistencia, y múltiples clientes en C++/Qt interactúan de forma asíncrona mediante protocolos HTTP.

---

## 🛠️ Stack Tecnológico

### **Infraestructura & Backend**
* **Hosting**: Servidor VPS (IP: `161.97.92.143`)
* **Contenedores**: Docker & Docker Compose
* **Servidor Web**: FastAPI (Python 3.11)
* **Base de Datos**: MySQL 8.0 (Relacional)
* **Administración**: phpMyAdmin para gestión de esquemas.

### **Frontend (Cliente Desktop)**
* **Framework**: Qt Framework (C++17)
* **Módulos**: `network`, `widgets`, `json`.
* **Estilo**: QSS (Qt Style Sheets) con diseño moderno y colores dinámicos.

---

## 🌟 Funcionalidades Principales

* **Sincronización Automática**: Sistema de *polling* que refresca el tablero cada 3 segundos sin bloquear la UI.
* **CRUD Full**: 
    * **Create**: Inserción de nuevas tareas con diálogos modales.
    * **Read**: Visualización de tarjetas clasificadas por categorías.
    * **Update**: Movimiento de tareas entre estados (➜) y edición de títulos.
    * **Delete**: Eliminación segura de tarjetas con confirmación (✕).
* **UI Dinámica**: Las tarjetas cambian de color automáticamente según su estado:
    * 🔴 **Pendiente**: `#ff7675` (Rojo pastel)
    * 🟡 **En Proceso**: `#ffeaa7` (Amarillo pastel)
    * 🟢 **Hecho**: `#55efc4` (Verde pastel)

---

## 🧠 Desafíos Técnicos Superados

1.  **Gestión de Memoria en C++**: Se implementó una limpieza de interfaz basada en `deleteLater()` y ocultamiento de widgets (`hide()`) para evitar errores de segmentación (*Segmentation Fault*) durante el redibujado asíncrono.
2.  **Concurrencia de Red**: Uso de `QNetworkAccessManager` para manejar peticiones asíncronas, evitando que la aplicación se "congele" mientras espera respuesta del servidor.
3.  **Cross-Origin & Seguridad**: Configuración de middlewares CORS en el backend para permitir la conexión de múltiples integrantes desde diferentes redes domésticas.

---

## 📂 Estructura de Archivos
```text
├── backend_AlcanIA/         # Root del proyecto en VPS
│   ├── ejercicio04_backend/
│   │   ├── main.py          # Lógica de la API REST
│   │   ├── Dockerfile       # Imagen de Python/FastAPI
│   │   └── requirements.txt # Librerías: pymysql, fastapi, etc.
│   └── docker-compose.yml   # Orquestación (API + MySQL + phpMyAdmin)
├── Ejercicio04_Qt/          # Proyecto en Qt Creator (C++)
│   ├── main.cpp
│   ├── mainwindow.cpp       # Lógica de red y UI
│   ├── mainwindow.h
│   └── Ejercicio04.pro      # Configuración del proyecto
```

---

## 📦 Ejecución
Para correr el cliente, abrir el archivo `.pro` en **Qt Creator**, realizar un **Run QMake** y ejecutar. La aplicación conectará automáticamente al endpoint de producción: `http://161.97.92.143:8001/tablero`.

---

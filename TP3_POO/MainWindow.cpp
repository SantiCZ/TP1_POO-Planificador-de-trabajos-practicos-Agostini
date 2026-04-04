#include "MainWindow.h"
#include <QToolBar>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QIcon>
#include <QFrame>
#include <QSettings>

// ── Color palette helper ──────────────────────────────────────────────────────

QColor MainWindow::colorForIndex(int idx) const {
    float t = (idx <= 0) ? 0.f : (idx >= 8) ? 1.f : idx / 8.f;
    int r = qRound(192 + (24  - 192) * t);
    int g = qRound(19  + (233 - 19)  * t);
    int b = qRound(76  + (199 - 76)  * t);
    return QColor(r, g, b);
}

// ── Constructor ───────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Collaborative Canvas");
    resize(1200, 800);

    m_model  = new DrawingModel(this);
    m_canvas = new CanvasView(m_model, this);
    m_sync   = new SyncManager(m_model, this);

    // Load server URL from settings (or prompt user)
    QSettings settings("CollabCanvas", "CollabCanvas");
    // Siempre preguntar la URL para evitar URLs corruptas cacheadas
    QString savedUrl = settings.value("serverUrl", "http://161.97.92.143:5005").toString();
    // Limpiar URLs que tengan rutas incorrectas (solo conservar host:puerto)
    QUrl parsedUrl(savedUrl);
    QString serverUrl = parsedUrl.scheme() + "://" + parsedUrl.host() + ":" + QString::number(parsedUrl.port(5005));

    bool ok;
    serverUrl = QInputDialog::getText(
        this, "Server URL",
        "Enter VPS server URL (e.g. http://161.97.92.143:5005):",
        QLineEdit::Normal, serverUrl, &ok);
    if (ok && !serverUrl.isEmpty()) {
        // Asegurarse de que no tenga barra al final ni rutas extra
        while (serverUrl.endsWith('/')) serverUrl.chop(1);
        settings.setValue("serverUrl", serverUrl);
    }
    m_sync->setServerUrl(serverUrl);

    setupToolbar();
    applyMetroStyle();

    setCentralWidget(m_canvas);

    // Status bar
    m_statusLabel = new QLabel("Ready");
    statusBar()->addWidget(m_statusLabel);

    // Connections
    connect(m_canvas, &CanvasView::thicknessChanged,
            this, &MainWindow::onThicknessChanged);
    connect(m_canvas, &CanvasView::strokeFinished, this, [this]() {
        // Auto-guardar al servidor cada vez que se termina un trazo
        m_sync->saveToServer();
    });
    connect(m_sync, &SyncManager::statusMessage, this, [this](const QString &msg) {
        m_statusLabel->setText(msg);
    });
    connect(m_sync, &SyncManager::saveSuccess,  this, &MainWindow::onSaveSuccess);
    connect(m_sync, &SyncManager::saveError,    this, &MainWindow::onSaveError);
    connect(m_sync, &SyncManager::fetchSuccess, this, [this]() {
        m_statusLabel->setText("Synced with server.");
    });

    // Initial fetch + start polling
    m_sync->fetchFromServer();
    m_sync->startPolling(2000);   // poll every 2 seconds para mayor inmediatez colaborativa

    // Set initial color
    m_canvas->setColor(colorForIndex(0));
    updateColorSwatch();
}

MainWindow::~MainWindow() {}

// ── Toolbar ───────────────────────────────────────────────────────────────────

void MainWindow::setupToolbar() {
    QToolBar *bar = addToolBar("Main");
    bar->setMovable(false);
    bar->setFloatable(false);
    bar->setIconSize(QSize(24, 24));
    bar->setObjectName("mainToolbar");

    // Save button (Metro style)
    m_saveButton = new QPushButton("  💾  GUARDAR");
    m_saveButton->setObjectName("metroSaveButton");
    m_saveButton->setFixedHeight(36);
    m_saveButton->setMinimumWidth(140);
    connect(m_saveButton, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    bar->addWidget(m_saveButton);

    bar->addSeparator();

    // Color swatch
    QLabel *colorLabel = new QLabel("Color:");
    colorLabel->setObjectName("toolLabel");
    bar->addWidget(colorLabel);

    m_colorSwatch = new QLabel();
    m_colorSwatch->setFixedSize(28, 28);
    m_colorSwatch->setObjectName("colorSwatch");
    bar->addWidget(m_colorSwatch);

    QLabel *keysHint = new QLabel("  Keys 1–9");
    keysHint->setObjectName("hintLabel");
    bar->addWidget(keysHint);

    bar->addSeparator();

    // Thickness indicator
    QLabel *thickLabel = new QLabel("Thickness:");
    thickLabel->setObjectName("toolLabel");
    bar->addWidget(thickLabel);

    m_thicknessLabel = new QLabel("6 px");
    m_thicknessLabel->setObjectName("thicknessLabel");
    m_thicknessLabel->setMinimumWidth(50);
    bar->addWidget(m_thicknessLabel);

    QLabel *scrollHint = new QLabel("  (scroll wheel)");
    scrollHint->setObjectName("hintLabel");
    bar->addWidget(scrollHint);

    bar->addSeparator();

    // Palette swatches (visual reference)
    QLabel *paletteLabel = new QLabel("Palette:");
    paletteLabel->setObjectName("toolLabel");
    bar->addWidget(paletteLabel);

    QWidget *palette = new QWidget();
    QHBoxLayout *pLayout = new QHBoxLayout(palette);
    pLayout->setContentsMargins(4, 2, 4, 2);
    pLayout->setSpacing(3);
    for (int i = 0; i < 9; ++i) {
        QLabel *swatch = new QLabel(QString::number(i+1));
        swatch->setFixedSize(22, 22);
        swatch->setAlignment(Qt::AlignCenter);
        QColor c = colorForIndex(i);
        swatch->setStyleSheet(QString(
                                  "background:%1; color:%2; font-size:10px; font-weight:bold; border:1px solid #555;")
                                  .arg(c.name())
                                  .arg(c.lightness() > 128 ? "#000" : "#fff"));
        pLayout->addWidget(swatch);
    }
    bar->addWidget(palette);

    bar->addSeparator();

    // Hints
    QLabel *hint = new QLabel("LMB: Draw  |  RMB: Erase");
    hint->setObjectName("hintLabel");
    bar->addWidget(hint);
}

void MainWindow::updateColorSwatch() {
    QColor c = colorForIndex(m_colorIndex);
    m_colorSwatch->setStyleSheet(QString(
                                     "background:%1; border:2px solid %2; border-radius:3px;")
                                     .arg(c.name())
                                     .arg(c.darker(150).name()));
}

// ── Styling ───────────────────────────────────────────────────────────────────

void MainWindow::applyMetroStyle() {
    setStyleSheet(R"(
        QMainWindow {
            background: #1a1a2e;
        }

        QToolBar#mainToolbar {
            background: #16213e;
            border-bottom: 2px solid #0f3460;
            padding: 4px 8px;
            spacing: 6px;
        }

        QPushButton#metroSaveButton {
            background: #c0134c;
            color: #ffffff;
            border: none;
            font-family: 'Segoe UI', 'Arial', sans-serif;
            font-size: 12px;
            font-weight: bold;
            letter-spacing: 2px;
            padding: 0 16px;
            text-transform: uppercase;
        }
        QPushButton#metroSaveButton:hover {
            background: #e01558;
        }
        QPushButton#metroSaveButton:pressed {
            background: #960f3a;
        }
        QPushButton#metroSaveButton:disabled {
            background: #555;
            color: #999;
        }

        QLabel#toolLabel {
            color: #8899bb;
            font-size: 11px;
            font-family: 'Segoe UI', sans-serif;
            padding: 0 4px;
        }
        QLabel#hintLabel {
            color: #556688;
            font-size: 10px;
            font-family: 'Segoe UI', sans-serif;
            padding: 0 4px;
        }
        QLabel#thicknessLabel {
            color: #18e9c7;
            font-size: 12px;
            font-weight: bold;
            font-family: 'Consolas', 'Courier New', monospace;
        }

        QToolBar QSeparator, QToolBar::separator {
            background: #0f3460;
            width: 1px;
            margin: 4px 6px;
        }

        QStatusBar {
            background: #16213e;
            color: #8899bb;
            font-size: 11px;
            font-family: 'Segoe UI', sans-serif;
            border-top: 1px solid #0f3460;
        }
    )");
}

// ── Key events (color selection) ──────────────────────────────────────────────

void MainWindow::keyPressEvent(QKeyEvent *event) {
    int key = event->key();
    if (key >= Qt::Key_1 && key <= Qt::Key_9) {
        m_colorIndex = key - Qt::Key_1;   // 0..8
        QColor c = colorForIndex(m_colorIndex);
        m_canvas->setColor(c);
        updateColorSwatch();
        m_statusLabel->setText(QString("Color %1: %2").arg(m_colorIndex+1).arg(c.name()));
    }
    QMainWindow::keyPressEvent(event);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void MainWindow::onSaveClicked() {
    m_saveButton->setEnabled(false);
    m_saveButton->setText("  ⏳  GUARDANDO…");
    m_sync->saveToServer();
}

void MainWindow::onThicknessChanged(int t) {
    m_thicknessLabel->setText(QString("%1 px").arg(t));
}

void MainWindow::onSaveSuccess() {
    // Solo restaurar el botón si fue deshabilitado (guardado manual)
    if (!m_saveButton->isEnabled()) {
        m_saveButton->setEnabled(true);
        m_saveButton->setText("  💾  GUARDAR");
    }
    m_statusLabel->setText("✓ Sincronizado");
}

void MainWindow::onSaveError(const QString &msg) {
    // Solo mostrar diálogo de error en guardado manual (botón deshabilitado)
    if (!m_saveButton->isEnabled()) {
        m_saveButton->setEnabled(true);
        m_saveButton->setText("  💾  GUARDAR");
        QMessageBox::warning(this, "Save Error", msg);
    } else {
        // Auto-guardado silencioso: solo mostrar en barra de estado
        m_statusLabel->setText("⚠ Error al sincronizar: " + msg);
    }
}
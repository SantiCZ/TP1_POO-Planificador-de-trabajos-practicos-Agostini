#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QScrollArea>
#include <QFrame>
#include <QApplication>
#include <QFont>
#include <QSizePolicy>
#include <QStatusBar>

// ──────────────────────────────────────────────
//  Constructor / Destructor
// ──────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("VPS Monitor"));
    setMinimumSize(920, 640);
    resize(1100, 720);

    applyDarkTheme();

    m_monitor = new ServerMonitor(this);

    // Temporizador de cuenta regresiva para el próximo chequeo
    m_countdownTimer = new QTimer(this);
    m_countdownTimer->setInterval(1000);
    connect(m_countdownTimer, &QTimer::timeout,
            this, &MainWindow::updateCountdownLabel);

    // Señales del monitor
    connect(m_monitor, &ServerMonitor::metricsUpdated,
            this, &MainWindow::onMetricsUpdated);
    connect(m_monitor, &ServerMonitor::connectionError,
            this, &MainWindow::onConnectionError);
    connect(m_monitor, &ServerMonitor::statusChanged,
            this, &MainWindow::onStatusChanged);
    connect(m_monitor, &ServerMonitor::newEventLogged,
            this, &MainWindow::onNewEventLogged);

    setupUi();
}

MainWindow::~MainWindow() {}

// ──────────────────────────────────────────────
//  Construcción del UI
// ──────────────────────────────────────────────
void MainWindow::setupUi()
{
    auto *central   = new QWidget(this);
    auto *mainVBox  = new QVBoxLayout(central);
    mainVBox->setContentsMargins(0, 0, 0, 0);
    mainVBox->setSpacing(0);
    setCentralWidget(central);

    // ── Top bar ──────────────────────────────
    auto *topBar = new QWidget(central);
    topBar->setFixedHeight(58);
    topBar->setObjectName(QStringLiteral("topBar"));
    setupTopBar(topBar);
    mainVBox->addWidget(topBar);

    // ── Config panel ─────────────────────────
    auto *cfgPanel = new QWidget(central);
    cfgPanel->setObjectName(QStringLiteral("configPanel"));
    cfgPanel->setFixedHeight(56);
    setupConfigPanel(cfgPanel);
    mainVBox->addWidget(cfgPanel);

    // ── Separador ────────────────────────────
    auto *sep = new QFrame(central);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QStringLiteral("background:#1E1E2E; border:none; min-height:1px; max-height:1px;"));
    mainVBox->addWidget(sep);

    // ── Área de contenido (métricas + log) ───
    auto *contentWidget = new QWidget(central);
    auto *contentHBox   = new QHBoxLayout(contentWidget);
    contentHBox->setContentsMargins(12, 12, 12, 12);
    contentHBox->setSpacing(12);

    // Panel izquierdo: métricas
    auto *metricsPanel = new QWidget(contentWidget);
    setupMetricsGrid(metricsPanel);

    // Panel derecho: log de eventos
    auto *logPanel = new QWidget(contentWidget);
    setupBottomPanel(logPanel);

    // Splitter para redimensionar
    auto *splitter = new QSplitter(Qt::Horizontal, contentWidget);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(metricsPanel);
    splitter->addWidget(logPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    splitter->setStyleSheet(QStringLiteral("QSplitter::handle { background: #1E1E2E; width: 2px; }"));

    contentHBox->addWidget(splitter);
    mainVBox->addWidget(contentWidget, 1);

    // Status bar
    statusBar()->setStyleSheet(QStringLiteral("background:#0E0E18; color:#555566; font-size:11px;"));
    statusBar()->showMessage(QStringLiteral("Listo. Configure el endpoint y presione Iniciar."));
}

void MainWindow::setupTopBar(QWidget *container)
{
    auto *hbox = new QHBoxLayout(container);
    hbox->setContentsMargins(16, 0, 16, 0);
    hbox->setSpacing(16);

    // Logotipo / título
    auto *titleLabel = new QLabel(QStringLiteral("⬡  VPS MONITOR"), container);
    titleLabel->setStyleSheet(QStringLiteral(
        "color:#7EC8E3; font-size:16px; font-weight:700; letter-spacing:2px;"));

    // Badge de estado
    m_statusBadge = new StatusBadge(container);
    m_statusBadge->setStatus(QStringLiteral("unknown"));
    m_statusBadge->setFixedWidth(110);

    // Uptime
    auto *uptimeLbl = new QLabel(QStringLiteral("Uptime:"), container);
    uptimeLbl->setStyleSheet(QStringLiteral("color:#555566; font-size:11px;"));
    m_uptimeLabel   = new QLabel(QStringLiteral("—"), container);
    m_uptimeLabel->setStyleSheet(QStringLiteral("color:#AAAACC; font-size:11px; font-weight:600;"));

    // Último chequeo
    auto *lastChkLbl = new QLabel(QStringLiteral("Último chequeo:"), container);
    lastChkLbl->setStyleSheet(QStringLiteral("color:#555566; font-size:11px;"));
    m_lastCheckLabel = new QLabel(QStringLiteral("—"), container);
    m_lastCheckLabel->setStyleSheet(QStringLiteral("color:#AAAACC; font-size:11px;"));

    // Countdown
    m_countdownLabel = new QLabel(QStringLiteral(""), container);
    m_countdownLabel->setStyleSheet(QStringLiteral("color:#444455; font-size:11px;"));

    hbox->addWidget(titleLabel);
    hbox->addSpacing(8);
    hbox->addWidget(m_statusBadge);
    hbox->addSpacerItem(new QSpacerItem(20, 1, QSizePolicy::Expanding));
    hbox->addWidget(uptimeLbl);
    hbox->addWidget(m_uptimeLabel);
    hbox->addSpacing(16);
    hbox->addWidget(lastChkLbl);
    hbox->addWidget(m_lastCheckLabel);
    hbox->addSpacing(8);
    hbox->addWidget(m_countdownLabel);
}

void MainWindow::setupConfigPanel(QWidget *container)
{
    auto *hbox = new QHBoxLayout(container);
    hbox->setContentsMargins(16, 6, 16, 6);
    hbox->setSpacing(10);

    // URL del endpoint
    // QLineEdit: widget de texto libre, necesario porque la URL es variable
    auto *urlLbl = new QLabel(QStringLiteral("Endpoint URL:"), container);
    urlLbl->setStyleSheet(QStringLiteral("color:#666677; font-size:11px;"));

    m_urlEdit = new QLineEdit(container);
    m_urlEdit->setPlaceholderText(QStringLiteral("https://mi-vps.com/api/health"));
    m_urlEdit->setMinimumWidth(280);
    m_urlEdit->setMaximumWidth(400);
    m_urlEdit->setStyleSheet(QStringLiteral(
        "QLineEdit { background:#1A1A26; border:1px solid #333344; border-radius:5px;"
        "            color:#DDDDEE; padding:4px 8px; font-size:12px; }"
        "QLineEdit:focus { border-color:#5577AA; }"));

    // Intervalo de chequeo
    // QSpinBox: valor entero acotado entre 5 y 3600 segundos. Ideal para este campo.
    auto *intLbl = new QLabel(QStringLiteral("Intervalo (s):"), container);
    intLbl->setStyleSheet(QStringLiteral("color:#666677; font-size:11px;"));

    m_intervalSpin = new QSpinBox(container);
    m_intervalSpin->setRange(5, 3600);
    m_intervalSpin->setValue(30);
    m_intervalSpin->setSuffix(QStringLiteral(" s"));
    m_intervalSpin->setFixedWidth(80);
    m_intervalSpin->setStyleSheet(QStringLiteral(
        "QSpinBox { background:#1A1A26; border:1px solid #333344; border-radius:5px;"
        "           color:#DDDDEE; padding:3px 6px; font-size:12px; }"
        "QSpinBox:focus { border-color:#5577AA; }"
        "QSpinBox::up-button, QSpinBox::down-button { background:#22223A; border:none; }"));

    // Umbral CPU
    // QDoubleSpinBox: permite decimales para afinar el umbral de alerta
    auto *cpuThrLbl = new QLabel(QStringLiteral("Umbral CPU%:"), container);
    cpuThrLbl->setStyleSheet(QStringLiteral("color:#666677; font-size:11px;"));

    m_threshCpuSpin = new QDoubleSpinBox(container);
    m_threshCpuSpin->setRange(10.0, 100.0);
    m_threshCpuSpin->setValue(80.0);
    m_threshCpuSpin->setSuffix(QStringLiteral(" %"));
    m_threshCpuSpin->setDecimals(1);
    m_threshCpuSpin->setFixedWidth(85);
    m_threshCpuSpin->setStyleSheet(m_intervalSpin->styleSheet());

    // Umbral RAM
    auto *memThrLbl = new QLabel(QStringLiteral("Umbral RAM%:"), container);
    memThrLbl->setStyleSheet(QStringLiteral("color:#666677; font-size:11px;"));

    m_threshMemSpin = new QDoubleSpinBox(container);
    m_threshMemSpin->setRange(10.0, 100.0);
    m_threshMemSpin->setValue(85.0);
    m_threshMemSpin->setSuffix(QStringLiteral(" %"));
    m_threshMemSpin->setDecimals(1);
    m_threshMemSpin->setFixedWidth(85);
    m_threshMemSpin->setStyleSheet(m_intervalSpin->styleSheet());

    // Botón Iniciar/Detener
    // QPushButton: acción principal de control del monitoreo
    m_startStopBtn = new QPushButton(QStringLiteral("▶  Iniciar"), container);
    m_startStopBtn->setFixedWidth(100);
    m_startStopBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background:#204030; color:#43C680; border:1px solid #2E5540;"
        "              border-radius:5px; padding:5px 10px; font-size:12px; font-weight:600; }"
        "QPushButton:hover { background:#2A5040; }"
        "QPushButton:pressed { background:#183428; }"));

    // Botón Refresco manual
    // QPushButton: permite al usuario forzar un chequeo inmediato sin esperar el intervalo
    m_refreshBtn = new QPushButton(QStringLiteral("⟳  Ahora"), container);
    m_refreshBtn->setFixedWidth(85);
    m_refreshBtn->setEnabled(false);
    m_refreshBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background:#1E2240; color:#7EC8E3; border:1px solid #2E3A55;"
        "              border-radius:5px; padding:5px 10px; font-size:12px; }"
        "QPushButton:hover { background:#263060; }"
        "QPushButton:disabled { color:#333344; border-color:#22223A; background:#141420; }"));

    hbox->addWidget(urlLbl);
    hbox->addWidget(m_urlEdit);
    hbox->addSpacing(8);
    hbox->addWidget(intLbl);
    hbox->addWidget(m_intervalSpin);
    hbox->addSpacing(8);
    hbox->addWidget(cpuThrLbl);
    hbox->addWidget(m_threshCpuSpin);
    hbox->addSpacing(4);
    hbox->addWidget(memThrLbl);
    hbox->addWidget(m_threshMemSpin);
    hbox->addStretch(1);
    hbox->addWidget(m_refreshBtn);
    hbox->addWidget(m_startStopBtn);

    // ── Conexiones ────────────────────────────
    connect(m_startStopBtn, &QPushButton::clicked, this, &MainWindow::onStartStopClicked);
    connect(m_refreshBtn,   &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_intervalSpin,  QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onIntervalChanged);
    connect(m_threshCpuSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onThresholdCpuChanged);
    connect(m_threshMemSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onThresholdMemChanged);
}

void MainWindow::setupMetricsGrid(QWidget *container)
{
    auto *vbox = new QVBoxLayout(container);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(10);

    // Título de sección
    auto *secLabel = new QLabel(QStringLiteral("MÉTRICAS DEL SERVIDOR"), container);
    secLabel->setStyleSheet(QStringLiteral(
        "color:#444455; font-size:10px; letter-spacing:2px; font-weight:700;"));

    // Grid 2×3 de tarjetas
    auto *grid = new QGridLayout();
    grid->setSpacing(10);

    // CPU – anillo, acento azul-cian
    m_cardCpu = new MetricCardWidget(QStringLiteral("CPU"),
                                     QStringLiteral("%"),
                                     MetricCardWidget::Style::Ring,
                                     container);
    m_cardCpu->setAccentColor(QColor(0x43, 0xB3, 0xD8));

    // RAM – anillo, acento verde
    m_cardMem = new MetricCardWidget(QStringLiteral("RAM"),
                                     QStringLiteral("%"),
                                     MetricCardWidget::Style::Ring,
                                     container);
    m_cardMem->setAccentColor(QColor(0x43, 0xC6, 0x80));

    // Disco – barra, acento naranja
    m_cardDisk = new MetricCardWidget(QStringLiteral("Disco"),
                                      QStringLiteral("%"),
                                      MetricCardWidget::Style::Bar,
                                      container);
    m_cardDisk->setAccentColor(QColor(0xF0, 0xA0, 0x30));

    // Red RX – barra
    m_cardRx = new MetricCardWidget(QStringLiteral("Red ↓ RX"),
                                    QStringLiteral("KB/s"),
                                    MetricCardWidget::Style::Bar,
                                    container);
    m_cardRx->setAccentColor(QColor(0x88, 0xBB, 0xFF));
    m_cardRx->setMaxValue(10240.0);   // 10 MB/s como máx visual

    // Red TX – barra
    m_cardTx = new MetricCardWidget(QStringLiteral("Red ↑ TX"),
                                    QStringLiteral("KB/s"),
                                    MetricCardWidget::Style::Bar,
                                    container);
    m_cardTx->setAccentColor(QColor(0xDD, 0x88, 0xFF));
    m_cardTx->setMaxValue(10240.0);

    grid->addWidget(m_cardCpu,  0, 0);
    grid->addWidget(m_cardMem,  0, 1);
    grid->addWidget(m_cardDisk, 0, 2);
    grid->addWidget(m_cardRx,   1, 0);
    grid->addWidget(m_cardTx,   1, 1);

    // Placeholder para celda vacía (futuras métricas)
    auto *placeholder = new QWidget(container);
    placeholder->setStyleSheet(QStringLiteral(
        "background:#131320; border:1px dashed #1E1E2E; border-radius:10px;"));
    grid->addWidget(placeholder, 1, 2);

    vbox->addWidget(secLabel);
    vbox->addLayout(grid, 1);
}

void MainWindow::setupBottomPanel(QWidget *container)
{
    auto *vbox = new QVBoxLayout(container);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(8);

    // Cabecera del log
    auto *hdr   = new QHBoxLayout();
    auto *lblLog = new QLabel(QStringLiteral("HISTORIAL DE EVENTOS"), container);
    lblLog->setStyleSheet(QStringLiteral(
        "color:#444455; font-size:10px; letter-spacing:2px; font-weight:700;"));

    // QPushButton: limpiar log, justificado porque el log puede acumular muchas entradas
    m_clearLogBtn = new QPushButton(QStringLiteral("Limpiar"), container);
    m_clearLogBtn->setFixedWidth(68);
    m_clearLogBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background:#1A1A26; color:#666677; border:1px solid #2A2A38;"
        "              border-radius:4px; padding:2px 8px; font-size:10px; }"
        "QPushButton:hover { color:#AAAACC; border-color:#444455; }"));

    hdr->addWidget(lblLog);
    hdr->addStretch(1);
    hdr->addWidget(m_clearLogBtn);

    // Widget de log
    m_eventLog = new EventLogWidget(container);

    vbox->addLayout(hdr);
    vbox->addWidget(m_eventLog, 1);

    connect(m_clearLogBtn, &QPushButton::clicked, this, &MainWindow::onClearLogClicked);
}

// ──────────────────────────────────────────────
//  Slots: acciones de usuario
// ──────────────────────────────────────────────
void MainWindow::onStartStopClicked()
{
    if (m_monitor->isRunning()) {
        m_monitor->stop();
        m_countdownTimer->stop();
        m_countdownLabel->setText(QString());
        m_startStopBtn->setText(QStringLiteral("▶  Iniciar"));
        m_startStopBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background:#204030; color:#43C680; border:1px solid #2E5540;"
            "              border-radius:5px; padding:5px 10px; font-size:12px; font-weight:600; }"
            "QPushButton:hover { background:#2A5040; }"));
        m_refreshBtn->setEnabled(false);
        statusBar()->showMessage(QStringLiteral("Monitoreo detenido."));
    } else {
        const QString url = m_urlEdit->text().trimmed();
        if (url.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("⚠ Ingrese una URL de endpoint válida."));
            return;
        }
        m_monitor->setEndpointUrl(url);
        m_monitor->setIntervalSeconds(m_intervalSpin->value());
        m_monitor->setWarningThresholdCpu(m_threshCpuSpin->value());
        m_monitor->setWarningThresholdMem(m_threshMemSpin->value());
        m_monitor->start();

        m_secondsToNext = m_intervalSpin->value();
        m_countdownTimer->start();

        m_startStopBtn->setText(QStringLiteral("■  Detener"));
        m_startStopBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background:#3A1020; color:#FF6E6E; border:1px solid #552030;"
            "              border-radius:5px; padding:5px 10px; font-size:12px; font-weight:600; }"
            "QPushButton:hover { background:#4A1828; }"));
        m_refreshBtn->setEnabled(true);
        statusBar()->showMessage(QStringLiteral("Monitoreo activo → ") + url);
    }
}

void MainWindow::onRefreshClicked()
{
    m_monitor->checkNow();
    m_secondsToNext = m_intervalSpin->value();
    statusBar()->showMessage(QStringLiteral("Chequeo manual solicitado…"));
}

void MainWindow::onClearLogClicked()
{
    m_eventLog->clearLog();
}

void MainWindow::onIntervalChanged(int secs)
{
    m_monitor->setIntervalSeconds(secs);
}

void MainWindow::onThresholdCpuChanged(double pct)
{
    m_monitor->setWarningThresholdCpu(pct);
    m_cardCpu->setWarningThreshold(pct);
}

void MainWindow::onThresholdMemChanged(double pct)
{
    m_monitor->setWarningThresholdMem(pct);
    m_cardMem->setWarningThreshold(pct);
}

void MainWindow::updateCountdownLabel()
{
    if (m_secondsToNext <= 0) {
        m_secondsToNext = m_intervalSpin->value();
    } else {
        m_secondsToNext--;
    }
    m_countdownLabel->setText(
        QStringLiteral("Próximo en %1s").arg(m_secondsToNext));
}

// ──────────────────────────────────────────────
//  Slots: datos del monitor
// ──────────────────────────────────────────────
void MainWindow::onMetricsUpdated(const ServerMetrics &m)
{
    m_statusBadge->setStatus(m.status);
    m_lastCheckLabel->setText(m.timestamp.toString(QStringLiteral("HH:mm:ss")));
    m_uptimeLabel->setText(m.uptime.isEmpty() ? QStringLiteral("—") : m.uptime);

    if (m.valid) {
        m_cardCpu->setValue(m.cpuLoad);
        m_cardMem->setValue(m.memUsedPct);
        m_cardDisk->setValue(m.diskUsedPct);
        m_cardRx->setValue(m.networkRxKbps);
        m_cardTx->setValue(m.networkTxKbps);

        m_cardCpu->setHistory(m_monitor->cpuHistory());
        m_cardMem->setHistory(m_monitor->memHistory());
    }

    m_secondsToNext = m_intervalSpin->value();

    statusBar()->showMessage(
        QStringLiteral("CPU: %1%  |  RAM: %2%  |  Disco: %3%  |  RX: %4 KB/s  |  TX: %5 KB/s")
            .arg(m.cpuLoad,    0, 'f', 1)
            .arg(m.memUsedPct, 0, 'f', 1)
            .arg(m.diskUsedPct,0, 'f', 1)
            .arg(m.networkRxKbps, 0, 'f', 1)
            .arg(m.networkTxKbps, 0, 'f', 1));
}

void MainWindow::onConnectionError(const QString &err)
{
    m_statusBadge->setStatus(QStringLiteral("down"));
    statusBar()->showMessage(QStringLiteral("❌ Error: ") + err);
}

void MainWindow::onStatusChanged(const QString &newStatus, const QString &/*oldStatus*/)
{
    m_statusBadge->setStatus(newStatus);
}

void MainWindow::onNewEventLogged(const EventEntry &e)
{
    m_eventLog->appendEvent(e);
}

// ──────────────────────────────────────────────
//  Tema oscuro global
// ──────────────────────────────────────────────
void MainWindow::applyDarkTheme()
{
    setStyleSheet(QStringLiteral(R"(
        QMainWindow, QWidget {
            background-color: #0E0E18;
            color: #CCCCDD;
            font-family: "Segoe UI", "Noto Sans", sans-serif;
        }
        QGroupBox {
            border: 1px solid #1E1E2E;
            border-radius: 8px;
            margin-top: 6px;
            padding-top: 10px;
            color: #555566;
            font-size: 10px;
            letter-spacing: 1px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
        }
        QSplitter {
            background: transparent;
        }
        QToolTip {
            background: #1A1A2A;
            color: #CCCCDD;
            border: 1px solid #333344;
            border-radius: 4px;
            padding: 4px;
        }
        #topBar {
            background: #0A0A14;
            border-bottom: 1px solid #1A1A28;
        }
        #configPanel {
            background: #0C0C1A;
            border-bottom: 1px solid #1A1A28;
        }
        QStatusBar {
            background: #0A0A14;
            color: #444455;
            font-size: 11px;
        }
    )"));
}

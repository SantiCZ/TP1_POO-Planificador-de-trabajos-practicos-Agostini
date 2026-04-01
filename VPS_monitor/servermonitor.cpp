#include "servermonitor.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>

// ──────────────────────────────────────────────
ServerMonitor::ServerMonitor(QObject *parent)
    : QObject(parent)
{
    m_nam   = new QNetworkAccessManager(this);
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);

    connect(m_nam,   &QNetworkAccessManager::finished,
            this,    &ServerMonitor::onNetworkReplyFinished);
    connect(m_timer, &QTimer::timeout,
            this,    &ServerMonitor::onTimerTick);
}

ServerMonitor::~ServerMonitor()
{
    stop();
}

// ── Configuración ─────────────────────────────
void ServerMonitor::setEndpointUrl(const QString &url)
{
    m_endpointUrl = url;
}

void ServerMonitor::setIntervalSeconds(int seconds)
{
    m_intervalSecs = qMax(5, seconds);
    if (m_timer->isActive()) {
        m_timer->setInterval(m_intervalSecs * 1000);
    }
}

void ServerMonitor::setWarningThresholdCpu(double pct)
{
    m_thresholdCpu = qBound(0.0, pct, 100.0);
}

void ServerMonitor::setWarningThresholdMem(double pct)
{
    m_thresholdMem = qBound(0.0, pct, 100.0);
}

// ── Control ───────────────────────────────────
void ServerMonitor::start()
{
    if (m_endpointUrl.isEmpty()) return;
    m_timer->start(m_intervalSecs * 1000);
    checkNow();   // primer chequeo inmediato
    addEvent(QStringLiteral("Monitoreo iniciado ▶"), QStringLiteral("info"));
}

void ServerMonitor::stop()
{
    m_timer->stop();
    if (m_activeReq) {
        m_activeReq->abort();
        m_activeReq = nullptr;
    }
    addEvent(QStringLiteral("Monitoreo detenido ■"), QStringLiteral("info"));
}

void ServerMonitor::checkNow()
{
    if (m_endpointUrl.isEmpty()) return;

    // Cancelar petición previa si aún está en vuelo
    if (m_activeReq) {
        m_activeReq->abort();
        m_activeReq = nullptr;
    }

    // 🔥 FIX DEL ERROR (most vexing parse)
    QNetworkRequest req{ QUrl(m_endpointUrl) };

    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setRawHeader("Accept", "application/json");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    m_activeReq = m_nam->get(req);
}

bool ServerMonitor::isRunning() const
{
    return m_timer->isActive();
}

// ── Slots privados ────────────────────────────
void ServerMonitor::onTimerTick()
{
    checkNow();
}

void ServerMonitor::onNetworkReplyFinished(QNetworkReply *reply)
{
    if (reply != m_activeReq) {
        reply->deleteLater();
        return;
    }
    m_activeReq = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        const QString errMsg = reply->errorString();
        addEvent(QStringLiteral("Error de red: ") + errMsg, QStringLiteral("critical"));

        // Marcar servidor como caído
        ServerMetrics down;
        down.status    = QStringLiteral("down");
        down.timestamp = QDateTime::currentDateTime();
        down.valid     = false;
        m_lastMetrics  = down;

        if (m_lastStatus != QStringLiteral("down")) {
            emit statusChanged(QStringLiteral("down"), m_lastStatus);
            m_lastStatus = QStringLiteral("down");
        }
        emit connectionError(errMsg);
        emit metricsUpdated(down);
    } else {
        parseReply(reply->readAll());
    }

    reply->deleteLater();
}

// ── Parseo JSON ───────────────────────────────
void ServerMonitor::parseReply(const QByteArray &data)
{
    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseErr);

    if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) {
        addEvent(QStringLiteral("Respuesta inválida: ") + parseErr.errorString(),
                 QStringLiteral("critical"));
        return;
    }

    QJsonObject obj = doc.object();

    ServerMetrics m;
    m.cpuLoad       = obj.value(QLatin1String("cpu_load")).toDouble();
    m.memUsedPct    = obj.value(QLatin1String("mem_used_pct")).toDouble();
    m.diskUsedPct   = obj.value(QLatin1String("disk_used_pct")).toDouble();
    m.networkRxKbps = obj.value(QLatin1String("network_rx_kbps")).toDouble();
    m.networkTxKbps = obj.value(QLatin1String("network_tx_kbps")).toDouble();
    m.uptime        = obj.value(QLatin1String("uptime")).toString(QStringLiteral("—"));
    m.timestamp     = QDateTime::currentDateTime();
    m.valid         = true;

    // Clasificar estado
    evaluateThresholds(m);

    // Historial
    m_cpuHistory.append(m.cpuLoad);
    m_memHistory.append(m.memUsedPct);
    while (m_cpuHistory.size() > HISTORY_SIZE) m_cpuHistory.removeFirst();
    while (m_memHistory.size() > HISTORY_SIZE) m_memHistory.removeFirst();

    // Cambio de estado
    if (m.status != m_lastStatus) {
        addEvent(QStringLiteral("Estado → ") + m.status.toUpper(), m.status);
        emit statusChanged(m.status, m_lastStatus);
        m_lastStatus = m.status;
    }

    m_lastMetrics = m;
    emit metricsUpdated(m);
}

void ServerMonitor::evaluateThresholds(ServerMetrics &m)
{
    if (m.cpuLoad >= m_thresholdCpu || m.memUsedPct >= m_thresholdMem) {
        if (m.cpuLoad >= 95.0 || m.memUsedPct >= 95.0) {
            m.status = QStringLiteral("critical");
        } else {
            m.status = QStringLiteral("warning");
        }
    } else {
        m.status = QStringLiteral("ok");
    }
}

void ServerMonitor::addEvent(const QString &msg, const QString &severity)
{
    EventEntry e;
    e.when     = QDateTime::currentDateTime();
    e.message  = msg;
    e.severity = severity;

    m_eventLog.prepend(e);
    if (m_eventLog.size() > 100) m_eventLog.removeLast();

    emit newEventLogged(e);
}
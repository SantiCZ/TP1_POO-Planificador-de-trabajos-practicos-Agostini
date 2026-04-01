#ifndef SERVERMONITOR_H
#define SERVERMONITOR_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDateTime>
#include <QJsonObject>
#include <QQueue>

// ─────────────────────────────────────────────
//  ServerMetrics  –  snapshot de salud del VPS
// ─────────────────────────────────────────────
struct ServerMetrics {
    double cpuLoad       = 0.0;   // % 0-100
    double memUsedPct    = 0.0;   // % 0-100
    double diskUsedPct   = 0.0;   // % 0-100
    double networkRxKbps = 0.0;   // KB/s
    double networkTxKbps = 0.0;   // KB/s
    QString uptime;               // e.g. "12d 4h 32m"
    QString status;               // "ok" | "warning" | "critical" | "down"
    QDateTime timestamp;
    bool valid = false;
};

// ─────────────────────────────────────────────
//  EventEntry  –  registro del historial
// ─────────────────────────────────────────────
struct EventEntry {
    QDateTime when;
    QString   message;
    QString   severity; // "info" | "warning" | "critical"
};

// ─────────────────────────────────────────────
//  ServerMonitor  –  lógica principal de polling
// ─────────────────────────────────────────────
class ServerMonitor : public QObject
{
    Q_OBJECT

public:
    explicit ServerMonitor(QObject *parent = nullptr);
    ~ServerMonitor();

    // Configuración
    void setEndpointUrl(const QString &url);
    void setIntervalSeconds(int seconds);
    void setWarningThresholdCpu(double pct);
    void setWarningThresholdMem(double pct);

    QString endpointUrl()       const { return m_endpointUrl; }
    int     intervalSeconds()   const { return m_intervalSecs; }
    double  thresholdCpu()      const { return m_thresholdCpu; }
    double  thresholdMem()      const { return m_thresholdMem; }

    // Control
    void start();
    void stop();
    void checkNow();          // Refresco manual

    // Datos
    ServerMetrics          lastMetrics()  const { return m_lastMetrics; }
    QList<EventEntry>      eventLog()     const { return m_eventLog; }
    QList<double>          cpuHistory()   const { return m_cpuHistory; }
    QList<double>          memHistory()   const { return m_memHistory; }

    bool isRunning() const;

signals:
    void metricsUpdated(const ServerMetrics &metrics);
    void connectionError(const QString &errorMsg);
    void statusChanged(const QString &newStatus, const QString &oldStatus);
    void newEventLogged(const EventEntry &entry);

private slots:
    void onTimerTick();
    void onNetworkReplyFinished(QNetworkReply *reply);

private:
    void parseReply(const QByteArray &data);
    void evaluateThresholds(ServerMetrics &m);
    void addEvent(const QString &msg, const QString &severity);
    QString classifyStatus(const ServerMetrics &m) const;

    // Red
    QNetworkAccessManager *m_nam       = nullptr;
    QNetworkReply         *m_activeReq = nullptr;

    // Timer
    QTimer *m_timer = nullptr;
    int     m_intervalSecs = 30;

    // Configuración
    QString m_endpointUrl;
    double  m_thresholdCpu = 80.0;
    double  m_thresholdMem = 85.0;

    // Estado
    ServerMetrics     m_lastMetrics;
    QString           m_lastStatus;
    QList<EventEntry> m_eventLog;

    // Historiales (últimas 60 muestras)
    static constexpr int HISTORY_SIZE = 60;
    QList<double> m_cpuHistory;
    QList<double> m_memHistory;
};

#endif // SERVERMONITOR_H

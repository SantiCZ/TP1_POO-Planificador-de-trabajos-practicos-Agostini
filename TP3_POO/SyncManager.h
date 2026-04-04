#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QImage>
#include "DrawingModel.h"

// Handles all VPS communication and collaborative sync
class SyncManager : public QObject {
    Q_OBJECT

public:
    explicit SyncManager(DrawingModel *model, QObject *parent = nullptr);

    void setServerUrl(const QString &url);
    void startPolling(int intervalMs = 4000);
    void stopPolling();

    // Save current model to VPS
    void saveToServer();

    // Fetch latest from VPS and merge
    void fetchFromServer();

signals:
    void saveSuccess();
    void saveError(const QString &msg);
    void fetchSuccess();
    void fetchError(const QString &msg);
    void statusMessage(const QString &msg);

private slots:
    void onSaveReply(QNetworkReply *reply);
    void onFetchReply(QNetworkReply *reply);
    void onPollTimer();

private:
    DrawingModel          *m_model;
    QNetworkAccessManager *m_nam;
    QTimer                *m_pollTimer;
    QString                m_serverUrl;

    bool m_saving  = false;
    bool m_fetching = false;
};

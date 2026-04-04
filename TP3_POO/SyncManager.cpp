#include "SyncManager.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>

SyncManager::SyncManager(DrawingModel *model, QObject *parent)
    : QObject(parent), m_model(model)
{
    m_nam = new QNetworkAccessManager(this);
    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &SyncManager::onPollTimer);
}

void SyncManager::setServerUrl(const QString &url) {
    m_serverUrl = url;
}

void SyncManager::startPolling(int intervalMs) {
    m_pollTimer->start(intervalMs);
}

void SyncManager::stopPolling() {
    m_pollTimer->stop();
}

// ── Save ──────────────────────────────────────────────────────────────────────

void SyncManager::saveToServer() {
    if (m_serverUrl.isEmpty()) {
        emit saveError("Server URL not configured.");
        return;
    }
    if (m_saving) return;
    m_saving = true;

    emit statusMessage("Saving to server…");

    QJsonDocument doc(m_model->toJson());
    QByteArray payload = doc.toJson(QJsonDocument::Compact);

    QNetworkRequest req(QUrl(m_serverUrl + "/drawing"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_nam->post(req, payload);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onSaveReply(reply);
    });
}

void SyncManager::onSaveReply(QNetworkReply *reply) {
    m_saving = false;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit saveError(reply->errorString());
        emit statusMessage("Save failed: " + reply->errorString());
        return;
    }
    emit saveSuccess();
    emit statusMessage("Saved successfully.");
}

// ── Fetch ─────────────────────────────────────────────────────────────────────

void SyncManager::fetchFromServer() {
    if (m_serverUrl.isEmpty()) return;
    if (m_fetching) return;
    m_fetching = true;

    QNetworkRequest req(QUrl(m_serverUrl + "/drawing"));
    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onFetchReply(reply);
    });
}

void SyncManager::onFetchReply(QNetworkReply *reply) {
    m_fetching = false;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit fetchError(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        emit fetchError("Invalid JSON response from server.");
        return;
    }

    QJsonObject obj = doc.object();
    QVector<Stroke> remoteStrokes;

    for (const auto &v : obj["strokes"].toArray())
        remoteStrokes.append(Stroke::fromJson(v.toObject()));

    // Incremental merge: only add new strokes, never lose local ones
    m_model->mergeStrokes(remoteStrokes);

    emit fetchSuccess();
}

// ── Polling ───────────────────────────────────────────────────────────────────

void SyncManager::onPollTimer() {
    fetchFromServer();
}
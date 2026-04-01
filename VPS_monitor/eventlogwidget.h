#ifndef EVENTLOGWIDGET_H
#define EVENTLOGWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include "servermonitor.h"

// ──────────────────────────────────────────────────────────
//  EventLogWidget
//  Panel de historial de eventos con colores por severidad.
//  Muestra la marca de tiempo, icono de severidad y mensaje.
// ──────────────────────────────────────────────────────────
class EventLogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EventLogWidget(QWidget *parent = nullptr);

public slots:
    void appendEvent(const EventEntry &entry);
    void loadAll(const QList<EventEntry> &entries);
    void clearLog();

private:
    QListWidget *m_list = nullptr;

    QColor colorForSeverity(const QString &sev) const;
    QString iconForSeverity(const QString &sev) const;
};

#endif // EVENTLOGWIDGET_H

#ifndef STATUSBADGE_H
#define STATUSBADGE_H

#include <QWidget>
#include <QString>

// ──────────────────────────────────────────────────────────
//  StatusBadge
//  Pastilla de estado visual (OK / WARNING / CRITICAL / DOWN)
//  con un indicador pulsante cuando el servidor está activo.
// ──────────────────────────────────────────────────────────
class StatusBadge : public QWidget
{
    Q_OBJECT

public:
    explicit StatusBadge(QWidget *parent = nullptr);

    void setStatus(const QString &status);   // "ok" | "warning" | "critical" | "down" | "unknown"
    QString status() const { return m_status; }

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    QString m_status   = QStringLiteral("unknown");
    int     m_timerId  = 0;
    float   m_pulse    = 0.0f;   // 0-1 ciclo de animación del dot

    struct Theme {
        QColor bg;
        QColor text;
        QColor dot;
        QString label;
    };
    Theme themeForStatus(const QString &s) const;
};

#endif // STATUSBADGE_H

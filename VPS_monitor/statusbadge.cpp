#include "statusbadge.h"

#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <cmath>

StatusBadge::StatusBadge(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(30);
    m_timerId = startTimer(50);  // ~20 fps para animación del pulse
}

void StatusBadge::setStatus(const QString &status)
{
    if (m_status != status) {
        m_status = status;
        update();
    }
}

QSize StatusBadge::sizeHint() const
{
    Theme t = themeForStatus(m_status);
    QFontMetrics fm(font());
    int textW = fm.horizontalAdvance(t.label);
    return QSize(textW + 52, 30);  // dot(10) + gaps + text + padding
}

void StatusBadge::timerEvent(QTimerEvent * /*event*/)
{
    m_pulse += 0.05f;
    if (m_pulse > 1.0f) m_pulse = 0.0f;
    update();
}

void StatusBadge::paintEvent(QPaintEvent * /*event*/)
{
    Theme t = themeForStatus(m_status);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // ── Pastilla de fondo ──────────────────────
    const QRectF rect(0, 0, width(), height());
    p.setBrush(t.bg);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect, height() / 2.0, height() / 2.0);

    // ── Punto pulsante ─────────────────────────
    const int dotX    = 14;
    const int dotY    = height() / 2;
    const int dotR    = 5;

    // Halo exterior pulsante
    float pulse = (m_status == QStringLiteral("ok") || m_status == QStringLiteral("warning"))
                      ? (0.5f + 0.5f * std::sin(m_pulse * 2.0f * 3.14159f))
                      : 0.0f;

    if (pulse > 0.01f) {
        QColor halo = t.dot;
        halo.setAlpha(static_cast<int>(80 * pulse));
        p.setBrush(halo);
        p.drawEllipse(QPoint(dotX, dotY), dotR + 3, dotR + 3);
    }

    p.setBrush(t.dot);
    p.drawEllipse(QPoint(dotX, dotY), dotR, dotR);

    // ── Texto del estado ───────────────────────
    p.setPen(t.text);
    QFont f = font();
    f.setPointSize(9);
    f.setBold(true);
    f.setLetterSpacing(QFont::AbsoluteSpacing, 0.8);
    p.setFont(f);

    QRect textRect(dotX + dotR + 6, 0, width() - dotX - dotR - 16, height());
    p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, t.label);
}

StatusBadge::Theme StatusBadge::themeForStatus(const QString &s) const
{
    if (s == QLatin1String("ok"))
        return { QColor(0x1A, 0x3B, 0x2A), QColor(0x4A, 0xD9, 0x8E), QColor(0x43, 0xC6, 0x80), QStringLiteral("OK") };
    if (s == QLatin1String("warning"))
        return { QColor(0x3A, 0x2E, 0x10), QColor(0xF5, 0xC2, 0x42), QColor(0xF0, 0xB4, 0x29), QStringLiteral("ALERTA") };
    if (s == QLatin1String("critical"))
        return { QColor(0x3A, 0x10, 0x10), QColor(0xFF, 0x5E, 0x5E), QColor(0xE0, 0x3C, 0x3C), QStringLiteral("CRÍTICO") };
    if (s == QLatin1String("down"))
        return { QColor(0x25, 0x25, 0x2E), QColor(0x88, 0x88, 0xAA), QColor(0x66, 0x66, 0x88), QStringLiteral("CAÍDO") };
    // unknown
    return { QColor(0x22, 0x22, 0x2E), QColor(0x77, 0x77, 0x88), QColor(0x55, 0x55, 0x66), QStringLiteral("—") };
}

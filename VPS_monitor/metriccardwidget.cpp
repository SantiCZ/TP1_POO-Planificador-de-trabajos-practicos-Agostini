#include "metriccardwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QFont>
#include <cmath>

MetricCardWidget::MetricCardWidget(const QString &title,
                                   const QString &unit,
                                   Style          style,
                                   QWidget       *parent)
    : QWidget(parent)
    , m_title(title)
    , m_unit(unit)
    , m_style(style)
{
    // La sparkline vive en la parte inferior de la card
    m_sparkline = new SparklineWidget(this);
    m_sparkline->setLabel(title);
    m_sparkline->setLineColor(m_accentColor);

    setMinimumSize(160, 200);
}

QSize MetricCardWidget::sizeHint() const
{
    return QSize(200, 220);
}

void MetricCardWidget::setValue(double value)
{
    m_value = value;
    update();
}

void MetricCardWidget::setHistory(const QList<double> &history)
{
    m_sparkline->setData(history);
    m_sparkline->setRange(0, m_maxValue);
    m_sparkline->setLineColor(colorForValue(m_value));
    update();
}

void MetricCardWidget::setWarningThreshold(double pct)
{
    m_warnThreshold = pct;
    update();
}

void MetricCardWidget::setCriticalThreshold(double pct)
{
    m_critThreshold = pct;
    update();
}

void MetricCardWidget::setAccentColor(const QColor &color)
{
    m_accentColor = color;
    m_sparkline->setLineColor(color);
    update();
}

void MetricCardWidget::setMaxValue(double max)
{
    m_maxValue = (max > 0) ? max : 1.0;
    m_sparkline->setRange(0, m_maxValue);
    update();
}

// ── Utilidades ────────────────────────────────
QColor MetricCardWidget::colorForValue(double v) const
{
    double pct = (m_maxValue > 0) ? v / m_maxValue * 100.0 : 0.0;
    if (pct >= m_critThreshold) return QColor(0xE0, 0x3C, 0x3C);
    if (pct >= m_warnThreshold) return QColor(0xF0, 0xB4, 0x29);
    return m_accentColor;
}

// ── Pintura principal ─────────────────────────
void MetricCardWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int W = width();
    const int H = height();

    // ── Fondo de la tarjeta ───────────────────
    QPainterPath bg;
    bg.addRoundedRect(0, 0, W, H, 10, 10);
    p.fillPath(bg, QColor(0x1C, 0x1C, 0x28));

    // ── Borde sutil ───────────────────────────
    p.setPen(QPen(QColor(0x33, 0x33, 0x44), 0.8));
    p.drawPath(bg);

    // ── Zona visual: anillo o barra ───────────
    const int sparkH   = 52;
    const int vizTop   = 10;
    const int vizBot   = H - sparkH - 10;
    const int vizH     = vizBot - vizTop;

    QRectF vizRect(10, vizTop, W - 20, vizH);

    if (m_style == Style::Ring) {
        drawRing(p, vizRect);
    } else {
        drawBar(p, vizRect);
    }

    // ── Posicionar sparkline ──────────────────
    m_sparkline->setGeometry(8, H - sparkH, W - 16, sparkH);
    m_sparkline->setLineColor(colorForValue(m_value));
}

// ── Anillo circular (CPU, RAM) ────────────────
void MetricCardWidget::drawRing(QPainter &p, const QRectF &rect)
{
    const double pct = qBound(0.0, m_value / m_maxValue * 100.0, 100.0);
    const QColor col = colorForValue(m_value);

    // Ring centrado
    const int margin  = 18;
    const int ringW   = qMin(static_cast<int>(rect.width()),
                             static_cast<int>(rect.height())) - margin * 2;
    const int ringH   = ringW;
    const QRectF ringRect(rect.x() + (rect.width()  - ringW) / 2.0,
                          rect.y() + (rect.height() - ringH) / 2.0,
                          ringW, ringH);
    const int penWidth = 8;

    // Track (fondo del anillo)
    p.setPen(QPen(QColor(0x2E, 0x2E, 0x3E), penWidth, Qt::SolidLine,
                  Qt::FlatCap, Qt::MiterJoin));
    p.setBrush(Qt::NoBrush);
    p.drawArc(ringRect.adjusted(penWidth / 2, penWidth / 2,
                                -penWidth / 2, -penWidth / 2),
              -90 * 16, -360 * 16);   // círculo completo para track

    // Arco de valor
    p.setPen(QPen(col, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
    int span = static_cast<int>(-pct / 100.0 * 360.0 * 16);
    p.drawArc(ringRect.adjusted(penWidth / 2, penWidth / 2,
                                -penWidth / 2, -penWidth / 2),
              90 * 16, span);

    // Valor central
    QRectF textRect = ringRect;
    QFont fVal = p.font();
    fVal.setPointSize(22);
    fVal.setBold(true);
    p.setFont(fVal);
    p.setPen(QColor(0xE8, 0xE8, 0xF2));
    p.drawText(textRect.adjusted(0, -8, 0, -8), Qt::AlignCenter,
               QString::number(static_cast<int>(m_value)));

    // Unidad debajo del valor
    QFont fUnit = p.font();
    fUnit.setPointSize(9);
    fUnit.setBold(false);
    p.setFont(fUnit);
    p.setPen(QColor(0x66, 0x66, 0x88));
    p.drawText(textRect.adjusted(0, 18, 0, 18), Qt::AlignCenter, m_unit);

    // Título sobre el anillo
    QFont fTitle = p.font();
    fTitle.setPointSize(9);
    fTitle.setBold(true);
    fTitle.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    p.setFont(fTitle);
    p.setPen(QColor(0x99, 0x99, 0xAA));
    p.drawText(QRectF(rect.x(), rect.y(), rect.width(), 18),
               Qt::AlignHCenter | Qt::AlignTop, m_title.toUpper());
}

// ── Barra horizontal (Disco, Red) ────────────
void MetricCardWidget::drawBar(QPainter &p, const QRectF &rect)
{
    const double pct = qBound(0.0, m_value / m_maxValue * 100.0, 100.0);
    const QColor col = colorForValue(m_value);

    // Título
    QFont fTitle = p.font();
    fTitle.setPointSize(9);
    fTitle.setBold(true);
    fTitle.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    p.setFont(fTitle);
    p.setPen(QColor(0x99, 0x99, 0xAA));
    p.drawText(QRectF(rect.x(), rect.y(), rect.width(), 18),
               Qt::AlignHCenter | Qt::AlignTop, m_title.toUpper());

    // Valor grande
    QFont fVal = p.font();
    fVal.setPointSize(28);
    fVal.setBold(true);
    p.setFont(fVal);
    p.setPen(QColor(0xE8, 0xE8, 0xF2));

    QString valStr;
    if (m_maxValue == 100.0) {
        valStr = QString::number(static_cast<int>(m_value)) + QStringLiteral("%");
    } else {
        // Kbps → Mbps si es grande
        if (m_value > 1024.0)
            valStr = QString::number(m_value / 1024.0, 'f', 1) + QStringLiteral(" M");
        else
            valStr = QString::number(static_cast<int>(m_value)) + QStringLiteral(" K");
    }

    QRectF valRect(rect.x(), rect.y() + 20, rect.width(), rect.height() - 60);
    p.drawText(valRect, Qt::AlignCenter, valStr);

    // Unidad
    QFont fUnit = p.font();
    fUnit.setPointSize(9);
    fUnit.setBold(false);
    p.setFont(fUnit);
    p.setPen(QColor(0x66, 0x66, 0x88));
    p.drawText(valRect.adjusted(0, 30, 0, 30), Qt::AlignHCenter | Qt::AlignTop, m_unit);

    // Barra de progreso
    const int barH    = 6;
    const int barY    = static_cast<int>(rect.bottom()) - barH - 4;
    const int barXmin = static_cast<int>(rect.x());
    const int barW    = static_cast<int>(rect.width());

    // Track
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0x2E, 0x2E, 0x3E));
    p.drawRoundedRect(barXmin, barY, barW, barH, barH / 2, barH / 2);

    // Relleno
    int fillW = static_cast<int>(pct / 100.0 * barW);
    if (fillW > 0) {
        p.setBrush(col);
        p.drawRoundedRect(barXmin, barY, fillW, barH, barH / 2, barH / 2);
    }
}

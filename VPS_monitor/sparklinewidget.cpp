#include "sparklinewidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QFontMetrics>

SparklineWidget::SparklineWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(48);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

void SparklineWidget::setData(const QList<double> &values)
{
    m_data = values;
    update();
}

void SparklineWidget::setRange(double min, double max)
{
    m_min = min;
    m_max = max;
    update();
}

void SparklineWidget::setLineColor(const QColor &color)
{
    m_color = color;
    update();
}

void SparklineWidget::setLabel(const QString &label)
{
    m_label = label;
    update();
}

void SparklineWidget::paintEvent(QPaintEvent * /*event*/)
{
    if (m_data.isEmpty()) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int W = width();
    const int H = height();
    const int n = m_data.size();

    // Reservar espacio para label en la parte inferior
    const int labelH = m_label.isEmpty() ? 0 : 14;
    const int chartH = H - labelH;

    // Padding vertical mínimo
    const double topPad    = 4.0;
    const double bottomPad = 4.0;
    const double range     = (m_max - m_min) > 0 ? (m_max - m_min) : 1.0;

    auto toX = [&](int i) -> double {
        return (n == 1) ? W / 2.0 : static_cast<double>(i) * (W - 1) / (n - 1);
    };
    auto toY = [&](double v) -> double {
        double norm = (v - m_min) / range;
        return topPad + (1.0 - norm) * (chartH - topPad - bottomPad);
    };

    // ── Construir path ────────────────────────
    QPainterPath linePath;
    QPainterPath fillPath;

    linePath.moveTo(toX(0), toY(m_data[0]));
    fillPath.moveTo(toX(0), chartH);
    fillPath.lineTo(toX(0), toY(m_data[0]));

    for (int i = 1; i < n; ++i) {
        double x0 = toX(i - 1), y0 = toY(m_data[i - 1]);
        double x1 = toX(i),     y1 = toY(m_data[i]);

        // Control points para suavizado tipo Bezier
        double cx = (x0 + x1) / 2.0;
        linePath.cubicTo(cx, y0, cx, y1, x1, y1);
        fillPath.cubicTo(cx, y0, cx, y1, x1, y1);
    }

    fillPath.lineTo(toX(n - 1), chartH);
    fillPath.closeSubpath();

    // ── Gradiente de relleno ──────────────────
    QLinearGradient grad(0, 0, 0, chartH);
    QColor colorTop = m_color;
    colorTop.setAlpha(100);
    QColor colorBot = m_color;
    colorBot.setAlpha(10);
    grad.setColorAt(0.0, colorTop);
    grad.setColorAt(1.0, colorBot);

    p.fillPath(fillPath, grad);

    // ── Línea ─────────────────────────────────
    QPen pen(m_color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    p.drawPath(linePath);

    // ── Punto actual (último valor) ────────────
    double lastX = toX(n - 1);
    double lastY = toY(m_data.last());
    p.setBrush(m_color);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(lastX, lastY), 3.0, 3.0);

    // ── Label ─────────────────────────────────
    if (!m_label.isEmpty()) {
        p.setPen(QColor(0x88, 0x88, 0x99));
        QFont f = font();
        f.setPointSize(8);
        p.setFont(f);
        p.drawText(0, chartH, W, labelH, Qt::AlignCenter, m_label);
    }
}

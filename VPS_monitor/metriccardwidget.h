#ifndef METRICCARDWIDGET_H
#define METRICCARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include "sparklinewidget.h"

// ──────────────────────────────────────────────────────────
//  MetricCardWidget
//  Tarjeta individual que muestra una métrica (CPU, RAM, Disco,
//  Red). Diseño inspirado en Grafana/Netdata:
//  - Título e ícono en la parte superior
//  - Valor grande en el centro con unidad
//  - Barra de progreso circular o lineal
//  - Sparkline del historial en la parte inferior
// ──────────────────────────────────────────────────────────
class MetricCardWidget : public QWidget
{
    Q_OBJECT

public:
    enum class Style { Ring, Bar };  // Estilo de indicador visual

    explicit MetricCardWidget(const QString &title,
                              const QString &unit,
                              Style          style = Style::Ring,
                              QWidget       *parent = nullptr);

    void setValue(double value);
    void setHistory(const QList<double> &history);
    void setWarningThreshold(double pct);
    void setCriticalThreshold(double pct);
    void setAccentColor(const QColor &color);
    void setMaxValue(double max);   // para unidades no % (Kbps, etc.)

    double value() const { return m_value; }

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    void drawRing(QPainter &p, const QRectF &rect);
    void drawBar(QPainter &p, const QRectF &rect);
    QColor colorForValue(double v) const;

    QString m_title;
    QString m_unit;
    Style   m_style;

    double  m_value           = 0.0;
    double  m_maxValue        = 100.0;
    double  m_warnThreshold   = 80.0;
    double  m_critThreshold   = 95.0;
    QColor  m_accentColor     = QColor(0x43, 0xB3, 0x8D);

    SparklineWidget *m_sparkline = nullptr;
};

#endif // METRICCARDWIDGET_H

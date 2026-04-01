#ifndef SPARKLINEWIDGET_H
#define SPARKLINEWIDGET_H

#include <QWidget>
#include <QList>
#include <QColor>

// ──────────────────────────────────────────────────────────
//  SparklineWidget
//  Dibuja una mini-gráfica de línea (estilo Grafana) con
//  un gradiente de relleno. Útil para visualizar el historial
//  de CPU o memoria sin ocupar demasiado espacio.
// ──────────────────────────────────────────────────────────
class SparklineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SparklineWidget(QWidget *parent = nullptr);

    void setData(const QList<double> &values);
    void setRange(double min, double max);
    void setLineColor(const QColor &color);
    void setLabel(const QString &label);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<double> m_data;
    double        m_min   = 0.0;
    double        m_max   = 100.0;
    QColor        m_color = QColor(0x43, 0xB3, 0x8D);  // teal por defecto
    QString       m_label;
};

#endif // SPARKLINEWIDGET_H

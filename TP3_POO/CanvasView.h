#pragma once

#include <QWidget>
#include <QImage>
#include <QColor>
#include <QTimer>
#include "DrawingModel.h"

class CanvasView : public QWidget {
    Q_OBJECT

public:
    explicit CanvasView(DrawingModel *model, QWidget *parent = nullptr);

    void setColor(const QColor &color);
    void setThickness(int t);
    int  thickness() const { return m_thickness; }

    // Load a remote image into the canvas (replaces base)
    void loadImage(const QImage &img);
    QImage currentImage() const { return m_canvasImage; }

    void rebuildCanvas();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void strokeFinished();
    void thicknessChanged(int t);

private:
    DrawingModel *m_model;

    QImage m_canvasImage;     // Accumulated finished strokes
    QImage m_overlayImage;    // Current in-progress stroke (live)

    QColor m_color;
    int    m_thickness = 6;
    bool   m_drawing   = false;
    bool   m_erasing   = false;

    // Palette: 9 colors interpolated between (192,19,76) → (24,233,199)
    QColor colorForIndex(int idx) const;   // idx 0..8

    void drawCurrentStrokeToOverlay();
    void bakeCurrentStroke();
    void initCanvas();
};

#include "CanvasView.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QtMath>

CanvasView::CanvasView(DrawingModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    setAttribute(Qt::WA_StaticContents);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    m_color = colorForIndex(0);

    connect(m_model, &DrawingModel::modelChanged, this, [this]() {
        rebuildCanvas();
        update();
    });
}

// ── Color palette ─────────────────────────────────────────────────────────────

QColor CanvasView::colorForIndex(int idx) const {
    // idx 0..8 → interpolate (192,19,76) → (24,233,199)
    float t = (idx <= 0) ? 0.f : (idx >= 8) ? 1.f : idx / 8.f;
    int r = qRound(192 + (24  - 192) * t);
    int g = qRound(19  + (233 - 19)  * t);
    int b = qRound(76  + (199 - 76)  * t);
    return QColor(r, g, b);
}

void CanvasView::setColor(const QColor &color) { m_color = color; }

void CanvasView::setThickness(int t) {
    m_thickness = qBound(1, t, 80);
    emit thicknessChanged(m_thickness);
}

// ── Canvas init / resize ──────────────────────────────────────────────────────

void CanvasView::initCanvas() {
    m_canvasImage  = QImage(size(), QImage::Format_RGB32);
    m_canvasImage.fill(Qt::white);
    m_overlayImage = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    m_overlayImage.fill(Qt::transparent);
}

void CanvasView::resizeEvent(QResizeEvent *event) {
    if (width() > m_canvasImage.width() || height() > m_canvasImage.height()) {
        int newW = qMax(width(),  m_canvasImage.width());
        int newH = qMax(height(), m_canvasImage.height());
        QImage newImg(newW, newH, QImage::Format_RGB32);
        newImg.fill(Qt::white);
        QPainter p(&newImg);
        p.drawImage(0, 0, m_canvasImage);
        m_canvasImage = newImg;

        QImage newOverlay(newW, newH, QImage::Format_ARGB32_Premultiplied);
        newOverlay.fill(Qt::transparent);
        m_overlayImage = newOverlay;
    }
    QWidget::resizeEvent(event);
}

void CanvasView::loadImage(const QImage &img) {
    m_canvasImage = img.convertToFormat(QImage::Format_RGB32)
                       .scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QImage full(size(), QImage::Format_RGB32);
    full.fill(Qt::white);
    QPainter p(&full);
    p.drawImage(0, 0, m_canvasImage);
    m_canvasImage = full;
    update();
}

void CanvasView::rebuildCanvas() {
    // Re-render everything from model strokes
    m_canvasImage = QImage(size(), QImage::Format_RGB32);
    m_canvasImage.fill(Qt::white);
    m_model->renderToImage(m_canvasImage);

    m_overlayImage = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    m_overlayImage.fill(Qt::transparent);
}

// ── Painting ──────────────────────────────────────────────────────────────────

static QVector<QPointF> catmullRom(const QVector<QPointF> &pts, int segs = 8) {
    if (pts.size() < 2) return pts;
    QVector<QPointF> out;
    out.reserve(pts.size() * segs);

    auto cr = [](QPointF p0, QPointF p1, QPointF p2, QPointF p3, float t) {
        float t2 = t*t, t3 = t2*t;
        return QPointF(
            0.5f*((2*p1.x())+(-p0.x()+p2.x())*t+(2*p0.x()-5*p1.x()+4*p2.x()-p3.x())*t2+(-p0.x()+3*p1.x()-3*p2.x()+p3.x())*t3),
            0.5f*((2*p1.y())+(-p0.y()+p2.y())*t+(2*p0.y()-5*p1.y()+4*p2.y()-p3.y())*t2+(-p0.y()+3*p1.y()-3*p2.y()+p3.y())*t3)
        );
    };

    out.append(pts.first());
    for (int i = 0; i < pts.size()-1; ++i) {
        QPointF p0 = pts[qMax(0, i-1)];
        QPointF p1 = pts[i];
        QPointF p2 = pts[i+1];
        QPointF p3 = pts[qMin((int)pts.size()-1, i+2)];
        for (int j = 1; j <= segs; ++j)
            out.append(cr(p0, p1, p2, p3, (float)j/segs));
    }
    return out;
}

void CanvasView::drawCurrentStrokeToOverlay() {
    Stroke *s = m_model->currentStroke();
    if (!s || s->points.isEmpty()) return;

    m_overlayImage.fill(Qt::transparent);
    QPainter painter(&m_overlayImage);
    painter.setRenderHint(QPainter::Antialiasing);

    auto smooth = catmullRom(s->points);

    if (s->isEraser) {
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        for (const auto &p : smooth)
            painter.drawEllipse(p, s->thickness/2.0, s->thickness/2.0);
    } else {
        QPen pen(s->color);
        pen.setWidth(s->thickness);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);

        if (smooth.size() == 1) {
            painter.drawPoint(smooth[0]);
        } else {
            QPainterPath path;
            path.moveTo(smooth[0]);
            for (int i = 1; i < smooth.size(); ++i)
                path.lineTo(smooth[i]);
            painter.drawPath(path);
        }
    }
}

void CanvasView::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.drawImage(0, 0, m_canvasImage);

    // Overlay: current stroke being drawn
    if (m_drawing || m_erasing) {
        if (m_erasing) {
            // For eraser overlay, composite it correctly
            painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        }
        painter.drawImage(0, 0, m_overlayImage);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    // Draw cursor indicator
    // (cursor is handled by Qt)
}

// ── Mouse events ──────────────────────────────────────────────────────────────

void CanvasView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_drawing = true;
        m_erasing = false;
        m_model->beginStroke(event->pos(), m_color, m_thickness, false);
    } else if (event->button() == Qt::RightButton) {
        m_erasing = true;
        m_drawing = false;
        m_model->beginStroke(event->pos(), Qt::white, m_thickness * 2, true);
    }
    drawCurrentStrokeToOverlay();
    update();
}

void CanvasView::mouseMoveEvent(QMouseEvent *event) {
    if (!m_drawing && !m_erasing) return;
    m_model->addPoint(event->pos());
    drawCurrentStrokeToOverlay();
    update();
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event) {
    if ((event->button() == Qt::LeftButton && m_drawing) ||
        (event->button() == Qt::RightButton && m_erasing)) {
        m_drawing = false;
        m_erasing = false;
        m_model->endStroke();
        m_overlayImage.fill(Qt::transparent);
        emit strokeFinished();
        update();
    }
}

void CanvasView::wheelEvent(QWheelEvent *event) {
    int delta = event->angleDelta().y() > 0 ? 2 : -2;
    setThickness(m_thickness + delta);
    event->accept();
}

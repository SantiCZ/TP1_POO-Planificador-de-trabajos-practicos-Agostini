#include "DrawingModel.h"
#include <QPainter>
#include <QPainterPath>
#include <QJsonDocument>
#include <QDateTime>
#include <cmath>

// ── Stroke serialization ─────────────────────────────────────────────────────

QJsonObject Stroke::toJson() const {
    QJsonObject obj;
    obj["id"] = id;
    obj["color"] = color.name(QColor::HexArgb);
    obj["thickness"] = thickness;
    obj["isEraser"] = isEraser;
    obj["timestamp"] = timestamp;

    QJsonArray pts;
    for (const auto &p : points) {
        QJsonObject pt;
        pt["x"] = p.x();
        pt["y"] = p.y();
        pts.append(pt);
    }
    obj["points"] = pts;
    return obj;
}

Stroke Stroke::fromJson(const QJsonObject &obj) {
    Stroke s;
    s.id        = obj["id"].toString();
    s.color     = QColor(obj["color"].toString());
    s.thickness = obj["thickness"].toInt(6);
    s.isEraser  = obj["isEraser"].toBool(false);
    s.timestamp = obj["timestamp"].toVariant().toLongLong();

    for (const auto &v : obj["points"].toArray()) {
        QJsonObject pt = v.toObject();
        s.points.append(QPointF(pt["x"].toDouble(), pt["y"].toDouble()));
    }
    return s;
}

// ── DrawingModel ─────────────────────────────────────────────────────────────

DrawingModel::DrawingModel(QObject *parent) : QObject(parent) {}

void DrawingModel::beginStroke(const QPointF &point, const QColor &color,
                               int thickness, bool isEraser) {
    m_currentStroke = new Stroke();
    m_currentStroke->color     = isEraser ? Qt::white : color;
    m_currentStroke->thickness = thickness;
    m_currentStroke->isEraser  = isEraser;
    m_currentStroke->timestamp = QDateTime::currentMSecsSinceEpoch();
    m_currentStroke->points.append(point);
}

void DrawingModel::addPoint(const QPointF &point) {
    if (!m_currentStroke) return;
    // Only add if moved enough (avoids duplicate points)
    if (!m_currentStroke->points.isEmpty()) {
        QPointF last = m_currentStroke->points.last();
        if (QLineF(last, point).length() < 2.0) return;
    }
    m_currentStroke->points.append(point);
    emit modelChanged();
}

void DrawingModel::endStroke() {
    if (!m_currentStroke) return;
    if (m_currentStroke->points.size() >= 1) {
        m_strokes.append(*m_currentStroke);
        emit strokeAdded(m_strokes.last());
    }
    delete m_currentStroke;
    m_currentStroke = nullptr;
    emit modelChanged();
}

void DrawingModel::mergeStrokes(const QVector<Stroke> &remoteStrokes) {
    // Build a set of known IDs
    QSet<QString> knownIds;
    for (const auto &s : m_strokes) knownIds.insert(s.id);

    bool changed = false;
    for (const auto &remote : remoteStrokes) {
        if (!knownIds.contains(remote.id)) {
            m_strokes.append(remote);
            knownIds.insert(remote.id);
            changed = true;
        }
    }

    if (changed) {
        // Sort by timestamp to preserve draw order
        std::sort(m_strokes.begin(), m_strokes.end(),
                  [](const Stroke &a, const Stroke &b) {
                      return a.timestamp < b.timestamp;
                  });
        emit modelChanged();
    }
}

QJsonObject DrawingModel::toJson() const {
    QJsonObject obj;
    QJsonArray arr;
    for (const auto &s : m_strokes) arr.append(s.toJson());
    obj["strokes"] = arr;
    return obj;
}

void DrawingModel::fromJson(const QJsonObject &obj) {
    m_strokes.clear();
    for (const auto &v : obj["strokes"].toArray())
        m_strokes.append(Stroke::fromJson(v.toObject()));
    emit modelChanged();
}

void DrawingModel::clear() {
    m_strokes.clear();
    delete m_currentStroke;
    m_currentStroke = nullptr;
    emit modelChanged();
}

// ── Catmull-Rom spline interpolation ─────────────────────────────────────────

QVector<QPointF> DrawingModel::catmullRomSpline(const QVector<QPointF> &pts,
                                                 int segments) const {
    QVector<QPointF> result;
    if (pts.size() < 2) return pts;
    if (pts.size() == 2) return pts;

    auto catmullRom = [](QPointF p0, QPointF p1, QPointF p2, QPointF p3, float t) {
        float t2 = t * t, t3 = t2 * t;
        return QPointF(
            0.5f * ((2*p1.x()) + (-p0.x()+p2.x())*t +
                    (2*p0.x()-5*p1.x()+4*p2.x()-p3.x())*t2 +
                    (-p0.x()+3*p1.x()-3*p2.x()+p3.x())*t3),
            0.5f * ((2*p1.y()) + (-p0.y()+p2.y())*t +
                    (2*p0.y()-5*p1.y()+4*p2.y()-p3.y())*t2 +
                    (-p0.y()+3*p1.y()-3*p2.y()+p3.y())*t3)
        );
    };

    result.append(pts.first());
    for (int i = 0; i < pts.size() - 1; ++i) {
        QPointF p0 = pts[qMax(0, i-1)];
        QPointF p1 = pts[i];
        QPointF p2 = pts[i+1];
        QPointF p3 = pts[qMin((int)pts.size()-1, i+2)];

        for (int j = 1; j <= segments; ++j) {
            float t = (float)j / segments;
            result.append(catmullRom(p0, p1, p2, p3, t));
        }
    }
    return result;
}

// ── Rendering ─────────────────────────────────────────────────────────────────

void DrawingModel::renderStroke(QPainter &painter, const Stroke &stroke) const {
    if (stroke.points.isEmpty()) return;

    painter.save();
    if (stroke.isEraser) {
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        // Draw eraser as circles along the path
        auto smooth = catmullRomSpline(stroke.points);
        for (const auto &p : smooth)
            painter.drawEllipse(p, stroke.thickness/2.0, stroke.thickness/2.0);
    } else {
        QPen pen(stroke.color);
        pen.setWidth(stroke.thickness);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing, true);

        auto smooth = catmullRomSpline(stroke.points);
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
    painter.restore();
}

void DrawingModel::renderToImage(QImage &image) const {
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    for (const auto &stroke : m_strokes)
        renderStroke(painter, stroke);
}

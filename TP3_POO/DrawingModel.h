#pragma once

#include <QObject>
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QImage>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>

// Represents a single stroke (pencil or eraser)
struct Stroke {
    QString id;           // Unique ID for merge support
    QVector<QPointF> points;
    QColor color;
    int thickness;
    bool isEraser;
    qint64 timestamp;     // For conflict resolution

    Stroke() : thickness(6), isEraser(false), timestamp(0) {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    QJsonObject toJson() const;
    static Stroke fromJson(const QJsonObject &obj);
};

class DrawingModel : public QObject {
    Q_OBJECT

public:
    explicit DrawingModel(QObject *parent = nullptr);

    // Stroke management
    void beginStroke(const QPointF &point, const QColor &color, int thickness, bool isEraser);
    void addPoint(const QPointF &point);
    void endStroke();

    // Incremental merge: adds strokes from remote that we don't have
    void mergeStrokes(const QVector<Stroke> &remoteStrokes);

    const QVector<Stroke> &strokes() const { return m_strokes; }
    Stroke *currentStroke() { return m_currentStroke; }

    // Serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &obj);

    // Render all strokes into an image
    void renderToImage(QImage &image) const;

    void clear();

signals:
    void strokeAdded(const Stroke &stroke);
    void modelChanged();

private:
    QVector<Stroke> m_strokes;
    Stroke *m_currentStroke = nullptr;

    void renderStroke(QPainter &painter, const Stroke &stroke) const;
    QVector<QPointF> catmullRomSpline(const QVector<QPointF> &pts, int segments = 8) const;
};

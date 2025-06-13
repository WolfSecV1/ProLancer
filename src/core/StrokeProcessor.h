#ifndef STROKEPROCESSOR_H
#define STROKEPROCESSOR_H

#include <QVector>
#include <QPoint>
#include "../data/Vertex.h"
#include "../data/StrokePoint.h"

class StrokeProcessor {

public:

    StrokeProcessor();

    QVector<QPointF> interpolatePoints(const QPointF& p1, const QPointF& p2, int segments);
    QVector<Vertex> generateVertices(const QVector<StrokePoint>& stroke);

};

#endif


#pragma once

#include <qvector.h>
#include "../data/StrokePoint.h"
#include "../data/Vertex.h"
#include "StrokeProcessor.h"


class StrokeManager {
public:
    StrokeManager();
    void addStroke(const QVector<StrokePoint>& stroke, StrokeProcessor& processor, QVector<Vertex>& vertices);
    void undo(StrokeProcessor& processor, QVector<Vertex>& vertices);
    void redo(StrokeProcessor& processor, QVector<Vertex>& vertices);
    void clear();
    void clearStrokeVertexCounts();
    void clearRedoStack();
    const QVector<QVector<StrokePoint>>& getStrokes() const;
    QVector<int> getStrokeVertexCounts();
    void setChangeSinceLastUndo(bool value);
    void appendToStrokes(const QVector<StrokePoint>& stroke);
//    bool canUndo() const;
private:
    QVector<QVector<StrokePoint>> strokes;
    QVector<int> strokeVertexCounts;
    QVector<QVector<StrokePoint>> strokeRedoList;
    bool changeSinceLastUndo = false;
};

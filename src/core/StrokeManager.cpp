#include "StrokeManager.h"
#include "StrokeProcessor.h"

StrokeManager::StrokeManager() {}

void StrokeManager::addStroke(const QVector<StrokePoint>& stroke, StrokeProcessor& processor, QVector<Vertex>& vertices) {
    
    strokes.append(stroke);

    auto newVertices = processor.generateVertices(stroke);
    vertices.append(newVertices);
    strokeVertexCounts.append(newVertices.size());
}

void StrokeManager::undo(StrokeProcessor& processor, QVector<Vertex>& vertices) {
    if (strokes.isEmpty()) return;

    changeSinceLastUndo = false;
    // Remove the last completed stroke
    strokeRedoList.append(strokes.back());
    strokes.pop_back();

    // Rebuild vertex buffer from remaining strokes
    vertices.clear();
    strokeVertexCounts.clear();

    // Generate vertices WITHOUT calling addStroke (to avoid recursion)
    for (const QVector<StrokePoint>& stroke : strokes) {
        auto newVertices = processor.generateVertices(stroke);
        vertices += newVertices;  // Or use operator+=
        strokeVertexCounts.append(newVertices.size());
    }
}

void StrokeManager::redo(StrokeProcessor& processor, QVector<Vertex>& vertices){
    if (strokeRedoList.isEmpty()) return;

    auto strokeToDo = strokeRedoList.back();
    strokeRedoList.pop_back();
    strokes.append(strokeToDo);

    vertices.clear();
    strokeVertexCounts.clear();

    // Generate vertices WITHOUT calling addStroke (to avoid recursion)
    for (const QVector<StrokePoint>& stroke : strokes) {
        auto newVertices = processor.generateVertices(stroke);
        vertices += newVertices;  // Or use operator+=
        strokeVertexCounts.append(newVertices.size());
    }
}

const QVector<QVector<StrokePoint>>& StrokeManager::getStrokes() const
{
    return strokes;
}

QVector<int> StrokeManager::getStrokeVertexCounts()
{
    return strokeVertexCounts;
}

void StrokeManager::setChangeSinceLastUndo(bool value){
    changeSinceLastUndo = value;
}

void StrokeManager::clearRedoStack(){
    strokeRedoList.clear();
}

void StrokeManager::appendToStrokes(const QVector<StrokePoint>& stroke)
{
    strokes.append(stroke);
}

void StrokeManager::clear() {
    strokes.clear();
}

void StrokeManager::clearStrokeVertexCounts() {
    strokeVertexCounts.clear();
}

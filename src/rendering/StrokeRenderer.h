#ifndef STROKERENDERER_H
#define STROKERENDERER_H

#include <qopenglfunctions.h> // Add this include to ensure QOpenGLFunctions is available
#include <qopenglbuffer.h>
#include "../data/Vertex.h"
#include <QVector>
#include <QColor>
#include <QPointF>

class StrokeRenderer : protected QOpenGLFunctions { // Inherit from QOpenGLFunctions to use initializeOpenGLFunctions
public:
    
    StrokeRenderer();

    void initialize(QOpenGLBuffer* vertexBuffer);
    void renderVertexBuffer(const QVector<Vertex>& vertices, const QVector<int>& strokeCounts, QOpenGLBuffer& buffer);
    void updateVertexBuffer(QOpenGLBuffer& buffer, const QVector<Vertex>& vertices);
    void clearBuffer(QOpenGLBuffer& buffer);

private:
    QOpenGLBuffer* vBuffer;
};

#endif

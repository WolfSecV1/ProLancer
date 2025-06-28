#include "StrokeRenderer.h"
#include <qopenglfunctions.h>
#include "../data/Vertex.h"
#include <cstddef>

StrokeRenderer::StrokeRenderer() : vBuffer(nullptr) {}  

void StrokeRenderer::initialize(QOpenGLBuffer* vertexBuffer) {  
    this->vBuffer = vertexBuffer;  
    initializeOpenGLFunctions();
}

void StrokeRenderer::renderVertexBuffer(const QVector<Vertex>& vertices, const QVector<int>& strokeCounts, QOpenGLBuffer& buffer)
{
    if (vertices.isEmpty()) return;

    if (!buffer.bind()) {
#ifdef QT_DEBUG
        qDebug() << "[renderVertexBuffer] Failed to bind buffer.";
#endif
        return;
    }

    // Sanity check vertex layout
    static_assert(sizeof(Vertex) == 6 * sizeof(float), "Vertex struct is not packed correctly");

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, x)));
    glColorPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, r)));

    int startVertex = 0;
    for (int i = 0; i < strokeCounts.size(); ++i) {
        int count = strokeCounts[i];
        if (count > 0) {
            glDrawArrays(GL_TRIANGLE_STRIP, startVertex, count);
            startVertex += count;
        }
    }

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    buffer.release();

#ifdef QT_DEBUG
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        qDebug() << "[renderVertexBuffer] GL Error:" << err;
    }
#endif
}


void StrokeRenderer::updateVertexBuffer(QOpenGLBuffer& buffer, const QVector<Vertex>& vertices) {
    if (!buffer.bind()) return;
    if (vertices.isEmpty()) {
        buffer.allocate(nullptr, 0);
    }
    else {
        buffer.allocate(vertices.data(), vertices.size() * sizeof(Vertex));
    }
    buffer.release();
}

void StrokeRenderer::clearBuffer(QOpenGLBuffer& buffer) {
    if (buffer.bind()) {
        buffer.allocate(nullptr, 0);
        buffer.release();
    }
}

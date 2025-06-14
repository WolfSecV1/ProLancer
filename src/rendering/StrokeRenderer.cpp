#include "StrokeRenderer.h"  
#include <qopenglfunctions.h>  
#include <iostream>  
#include "../data/Vertex.h"  
#include "../core/math/mathUtils.h"  

StrokeRenderer::StrokeRenderer() : vBuffer(nullptr) {}  

void StrokeRenderer::initialize(QOpenGLBuffer* vertexBuffer) {  
    this->vBuffer = vertexBuffer;  
    initializeOpenGLFunctions();
}  

void StrokeRenderer::renderStroke(const QVector<StrokePoint>& stroke, const QColor& color) {  
    QVector<Vertex> tempVertices;  

    for (int i = 0; i < stroke.size() - 1; ++i) {  
        const StrokePoint& p1 = stroke[i];  
        const StrokePoint& p2 = stroke[i + 1];  

        // Simple line segment without over-interpolation  
        QVector<QPointF> points;  
        points.append(p1.pos);  
        points.append(p2.pos);  

        for (int j = 0; j < points.size() - 1; ++j) {  
            QPointF currentPos = points[j];  
            QPointF nextPos = points[j + 1];  

            // Calculate direction  
            float dx = nextPos.x() - currentPos.x();  
            float dy = nextPos.y() - currentPos.y();  
            float len = std::sqrt(dx * dx + dy * dy);  

            if (len < 0.1f) continue; // Skip very short segments  

            dx /= len;  
            dy /= len;  

            // Use smaller thickness for smoother appearance  
            float thick = std::min<float>(p1.thickness, 3.0f) * 0.5f; // Reduce thickness  
            // Perpendicular offset  
            float perpX = -dy * thick;  
            float perpY = dx * thick;  

            float cx, cy;  
            cx = currentPos.x();
            cy = currentPos.y();

            Vertex v1 = { cx + perpX, cy + perpY, color.redF(), color.greenF(), color.blueF(), thick };  
            Vertex v2 = { cx - perpX, cy - perpY, color.redF(), color.greenF(), color.blueF(), thick };  

            tempVertices.append(v1);  
            tempVertices.append(v2);  
        }  
    }  

    // Render
    if (!tempVertices.isEmpty()) {  
        glBegin(GL_TRIANGLE_STRIP);  
        for (const Vertex& v : tempVertices) {  
            glColor4f(v.r, v.g, v.b, 0.8f);  
            glVertex2f(v.x, v.y);  
        }  
        glEnd();  
    }  
}

void StrokeRenderer::renderVertexBuffer(const QVector<Vertex>& vertices, const QVector<int>& strokeCounts, QOpenGLBuffer& buffer)
{
    if (vertices.isEmpty() || !buffer.bind()) return;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(0));
    glColorPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(2 * sizeof(float)));

    // Draw each stroke separately
    int startVertex = 0;
    for (int count : strokeCounts) {
        if (count > 0) {
            glDrawArrays(GL_TRIANGLE_STRIP, startVertex, count);
            startVertex += count;
        }
    }

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    buffer.release();
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
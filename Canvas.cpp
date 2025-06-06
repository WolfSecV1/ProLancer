#pragma comment(lib, "opengl32.lib")
#include "Canvas.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <iostream>
#include <cmath>
#include <algorithm>


// Canvas implementation TODO: Convert all of this to QOpenGLWindow
Canvas::Canvas(QWidget* parent) : QOpenGLWidget(parent), drawing(false), vboUpdateFlag(false)
{
    setMinimumSize(500, 500);
}

Canvas::~Canvas()
{
    makeCurrent();  // Ensure OpenGL context is current
    vBuffer.destroy();
}

void Canvas::initializeGL()
{
    //Init OpenGL Funcs
    initializeOpenGLFunctions();

    //Make bg white
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    //Antialiasing
    glEnable(GL_LINE_SMOOTH); //AntiAliasing
    glEnable(GL_BLEND); // Enables blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Blends color with transparency

    // Set Line width
    glLineWidth(3.0f);

    if (!vBuffer.isCreated()) {
        vBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        vBuffer.create(); 

        if (!vBuffer.isCreated()) {
            qWarning() << "Failed to create vertex buffer!";
        }
    }

    std::cout << "OpenGL Online" << std::endl;
}

void Canvas::paintGL()
{
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);

    // Check VBO
    if (vboUpdateFlag) {
        updateVertexBuffer();
        vboUpdateFlag = false;
    }

    // Render all strokes from buffer
    if (!vertices.isEmpty()) {
        renderVertexBuffer();
    }

    // Render current stroke being drawn (immediate mode)
    if (!currentStroke.isEmpty() && currentStroke.size() > 1) {
        renderCurrentStroke();
    }
}
void Canvas::renderCurrentStroke() {
    QVector<Vertex> tempVertices;

    for (int i = 0; i < currentStroke.size() - 1; ++i) {
        const StrokePoint& p1 = currentStroke[i];
        const StrokePoint& p2 = currentStroke[i + 1];

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
            float thick = std::min(p1.thickness, 3.0f) * 0.5f; // Reduce thickness

            // Perpendicular offset
            float perpX = -dy * thick;
            float perpY = dx * thick;

            float cx, cy;
            convertToOpenGLCoords(currentPos, cx, cy);

            Vertex v1 = { cx + perpX, cy + perpY, currentColor.redF(), currentColor.greenF(), currentColor.blueF(), thick };
            Vertex v2 = { cx - perpX, cy - perpY, currentColor.redF(), currentColor.greenF(), currentColor.blueF(), thick };

            tempVertices.append(v1);
            tempVertices.append(v2);
        }
    }

    // Render current stroke
    if (!tempVertices.isEmpty()) {
        glBegin(GL_TRIANGLE_STRIP);
        for (const Vertex& v : tempVertices) {
            glColor4f(v.r, v.g, v.b, 0.8f);
            glVertex2f(v.x, v.y);
        }
        glEnd();
    }
}

void Canvas::resizeGL(int width, int height) {
    glViewport(0, 0, width, height); // Set viewport

    glMatrixMode(GL_PROJECTION); // Switch to projection matrix stack
    glLoadIdentity(); // Reset Current Matrix to Identity matrix 
    glOrtho(0, width, height, 0, -1, 1); // Sets up Orthographic projection, Y is flipped because we are converting gl coords to Qt coords

    glMatrixMode(GL_MODELVIEW); // Matrix Ops affect model-voew matrix
    glLoadIdentity(); // Resert Current Matrix

    std::cout << "Resize GL: " << width << "x" << height << std::endl;
}

QVector<QPointF> Canvas::interpolatePoints(const QPointF& p1, const QPointF& p2, int n_S)
{
    QVector<QPointF> result; //Create empty stroke
    result.append(p1); //start with first point
    for (int i = 0; i <= n_S; ++i) { //loop for # of segments
        float t = static_cast<float>(i) / n_S; // t = i/n
        
        //Apply LERP formula to our new point
        QPointF interpolated(
            p1.x() * (1 - t) + p2.x() * t,
            p1.y() * (1 - t) + p2.y() * t
        );
        result.append(interpolated); //Append it
    }
    result.append(p2); // Last but not lease attach the last point
    return result; //Return the stroke
}



float Canvas::calculatePressure(const QPointF& posF, const QPointF& posI, qint64 deltaT) {  
    if (deltaT <= 0) {  
        return 0.5f;  
    }  
    float dx = posF.x() - posI.x();  
    float dy = posF.y() - posI.y();  
    float dist = std::sqrt(dx * dx + dy * dy);  
    float sec = deltaT / 1000.0f;  
    float speed = dist / sec;  
    float maxSpeed = 1000.0f;  
    float pressure = 1.0f - (speed / maxSpeed) * speedSensitivity;  
    if (pressure < 0.1f) {pressure = 0.1f;}
    if (pressure > 1.0f) {pressure = 1.0f;}
    return pressure;
}

// Fixed addStrokeToVertexBuffer
void Canvas::addStrokeToVertexBuffer(const QVector<StrokePoint>& stroke)
{
    if (stroke.size() < 2) return;

    int startVertexCount = vertices.size();

    for (int i = 0; i < stroke.size() - 1; ++i) {
        const StrokePoint& p1 = stroke[i];
        const StrokePoint& p2 = stroke[i + 1];

        // Reduce interpolation - only interpolate if points are far apart
        float dx = p2.pos.x() - p1.pos.x();
        float dy = p2.pos.y() - p1.pos.y();
        float distance = std::sqrt(dx * dx + dy * dy);

        QVector<QPointF> points;
        if (distance > 5.0f) {
            // Only interpolate if points are far apart
            points = interpolatePoints(p1.pos, p2.pos, 1); // Reduce interpolation
        }
        else {
            points.append(p1.pos);
            points.append(p2.pos);
        }

        for (int j = 0; j < points.size() - 1; ++j) {
            QPointF currentPos = points[j];
            QPointF nextPos = points[j + 1];

            // Calculate direction
            float dirX = nextPos.x() - currentPos.x();
            float dirY = nextPos.y() - currentPos.y();
            float len = std::sqrt(dirX * dirX + dirY * dirY);

            if (len < 0.1f) continue;

            dirX /= len;
            dirY /= len;

            // Interpolate thickness
            float t = static_cast<float>(j) / (points.size() - 1);
            float thick = p1.thickness * (1.0f - t) + p2.thickness * t;
            thick = std::min(thick, 4.0f) * 0.5f; // Limit and reduce thickness

            // Perpendicular offset
            float perpX = -dirY * thick;
            float perpY = dirX * thick;

            float cx, cy;
            convertToOpenGLCoords(currentPos, cx, cy);

            Vertex v1 = { cx + perpX, cy + perpY, currentColor.redF(), currentColor.greenF(), currentColor.blueF(), thick };
            Vertex v2 = { cx - perpX, cy - perpY, currentColor.redF(), currentColor.greenF(), currentColor.blueF(), thick };

            vertices.append(v1);
            vertices.append(v2);
        }
    }

    int newVertexCount = vertices.size() - startVertexCount;
    strokeVertexCounts.append(newVertexCount);
}

void Canvas::updateVertexBuffer() {
    if (!vBuffer.bind()) return;
    if (vertices.isEmpty()) {
        vBuffer.allocate(nullptr, 0);
    }
    else {
        vBuffer.allocate(vertices.data(), vertices.size() * sizeof(Vertex));
    }
    vBuffer.release();
}

void Canvas::renderVertexBuffer()
{
    if (vertices.isEmpty() || !vBuffer.bind()) return;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(0));
    glColorPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(2 * sizeof(float)));

    // Draw each stroke separately
    int startVertex = 0;
    for (int count : strokeVertexCounts) {
        if (count > 0) {
            glDrawArrays(GL_TRIANGLE_STRIP, startVertex, count);
            startVertex += count;
        }
    }

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    vBuffer.release();
}

void Canvas::rebuildVertexBuffer() {
    vertices.clear();
    for (const QVector<StrokePoint>& stroke : strokeList) {
        addStrokeToVertexBuffer(stroke);
    }
}

void Canvas::convertToOpenGLCoords(const QPointF& qtPoint, float& x, float& y) {
    x = static_cast<float>(qtPoint.x());
    y = static_cast<float>(qtPoint.y());
}

void Canvas::clearCanvas() {
    currentStroke.clear();
    strokeList.clear();
    vertices.clear();
    strokeVertexCounts.clear();

    // Clear the VBO
    if (vBuffer.bind()) {
        vBuffer.allocate(nullptr, 0);
        vBuffer.release();
    }

    update();
}

void Canvas::undo() {  
    if (currentStroke.isEmpty() && !strokeList.isEmpty()) {  

        strokeList.pop_back();
        
        vertices.clear();
        strokeVertexCounts.clear();

        for (const QVector<StrokePoint>& stroke : strokeList) {
            addStrokeToVertexBuffer(stroke); // This appends to vertices and strokeVertexCounts
        }

        updateVertexBuffer();

        update();  
    }  
}

void Canvas::setColor(const QColor& color) {
    currentColor = color;
}

void Canvas::setBrushOptions(float minT, float maxT, float s) {
    minThickness = minT;
    maxThickness = maxT;
    speedSensitivity = s;
}

void Canvas::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        drawing = true;
        currentStroke.clear();

        StrokePoint point;
        point.pos = event->pos();
        point.pressure = 0.2f;  // Start with higher pressure
        point.thickness = minThickness + (maxThickness - minThickness) * point.pressure;
        point.strokeTime = QTime::currentTime();

        currentStroke.append(point);
        timer.restart();
        update();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent* event)
{
    if (drawing && (event->buttons() & Qt::LeftButton)) {
        QPointF newPos = event->pos();

        // Only add point if it's moved enough (reduces oversensitivity)
        if (!currentStroke.isEmpty()) {
            QPointF lastPos = currentStroke.last().pos;
            float dx = newPos.x() - lastPos.x();
            float dy = newPos.y() - lastPos.y();
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < 1.5f) return; // Skip if movement is too small
        }

        StrokePoint point;
        point.pos = newPos;
        point.strokeTime = QTime::currentTime();

        // Simpler pressure calculation - less sensitive
        if (!currentStroke.isEmpty()) {
            const StrokePoint& lastPoint = currentStroke.last();
            qint64 timeDelta = lastPoint.strokeTime.msecsTo(point.strokeTime);
            point.pressure = calculatePressure(point.pos, lastPoint.pos, timeDelta);
            // Smooth pressure changes
            point.pressure = lastPoint.pressure * 0.25f + point.pressure * 0.75f;
        }
        else {
            point.pressure = 0.25f;
        }

        point.thickness = minThickness + (maxThickness - minThickness) * point.pressure;
        currentStroke.append(point);
        update();
    }
}


void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && drawing) {
        drawing = false;
        if (currentStroke.size() > 1) {
            addStrokeToVertexBuffer(currentStroke);
            updateVertexBuffer();
            strokeList.append(currentStroke);
        }
        currentStroke.clear();
        update();
    }
}
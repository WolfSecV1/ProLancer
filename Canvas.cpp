#pragma comment(lib, "opengl32.lib")
#include "Canvas.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <iostream>
#include <cmath>


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
    renderVertexBuffer();

    // For now, all points are drawn as connected points
    if (!currentStroke.isEmpty()) { // Dont render anything until user starts drawing
        
        glBegin(GL_QUADS); // We will define a bunch of 4-point rectangles to simulate small segment of the brush stroke

        for (int i = 0; i < currentStroke.size() - 1; ++i) { // Loop through each pair of consecutive stroke points

            //init both points
            const StrokePoint& p1 = currentStroke[i];
            const StrokePoint& p2 = currentStroke[i + 1];

            //smooth out points by interpolating them
            QVector<QPointF> interpolated = interpolatePoints(p1.pos, p2.pos); //interpolate the stroke

            for (int j = 0; j < interpolated.size() - 1; ++j) { //loop through interpolated points in pairs
                
                float x1, y1, x2, y2; //initialize points

                convertToOpenGLCoords(interpolated[j], x1, y1); //convert point 1 to OpenGL coords
                convertToOpenGLCoords(interpolated[j + 1], x2, y2); //convert point 2 to OpenGL coords

                //interpolate brush thickness
                float t = static_cast<float>(j) / (interpolated.size() - 1);
                float thick = p1.thickness * (1.0f - t) + p2.thickness * t;

                // Find Vector direction
                float dx = x2 - x1;
                float dy = y2 - y1;

                // Normalize
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 0) {
                    dx /= len;
                    dy /= len;
                }

                // Perpendicular offset
                float perpX = -dy * thick * 0.5f;
                float perpY = dx * thick * 0.5f;

                //set color with transparency
                glColor4f(currentColor.redF(), currentColor.greenF(), currentColor.blueF(), 0.8f);

                //draw rect
                glVertex2f(x1 + perpX, y1 + perpY);
                glVertex2f(x1 - perpX, y1 - perpY);
                glVertex2f(x2 - perpX, y2 - perpY);
                glVertex2f(x2 + perpX, y2 + perpY);
            }
        }
        glEnd();
    }

    std::cout << "Paint GL - " << strokeList.size() << " strokes, current: " << currentStroke.size() << " points" << std::endl;
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

void Canvas::addStrokeToVertexBuffer(const QVector<StrokePoint>& stroke)
{
    for (int i = 0; i < stroke.size() - 1; ++i) {
        const StrokePoint& p1 = currentStroke[i];
        const StrokePoint& p2 = currentStroke[i + 1];

        //smooth out points by interpolating them
        QVector<QPointF> interpolated = interpolatePoints(p1.pos, p2.pos); //interpolate the stroke

        for (int j = 0; j < interpolated.size() - 1; ++j) { //loop through interpolated points in pairs

            float x1, y1, x2, y2; //initialize points

            convertToOpenGLCoords(interpolated[j], x1, y1); //convert point 1 to OpenGL coords
            convertToOpenGLCoords(interpolated[j + 1], x2, y2); //convert point 2 to OpenGL coords

            //interpolate brush thickness
            float t = static_cast<float>(j) / (interpolated.size() - 1);
            float thick = p1.thickness * (1.0f - t) + p2.thickness * t;

            // Find Vector direction
            float dx = x2 - x1;
            float dy = y2 - y1;

            // Normalize
            float len = std::sqrt(dx * dx + dy * dy);
            if (len > 0) {
                dx /= len;
                dy /= len;
            }

            // Perpendicular offset
            float pX = -dy * thick * 0.5f;
            float pY = dx * thick * 0.5f;

            // 
            Vertex v1 = { x1 + pX, y1 + pY, currentColor.redF(), currentColor.greenF(), currentColor.blueF(), thick };
            Vertex v2 = { x1 - pX, y1 - pY, currentColor.redF(), currentColor.greenF(), currentColor.blueF(), thick };
            Vertex v3 = { x2 - pX, y2 - pY, currentColor.redF(), currentColor.greenF(), currentColor.blueF(), thick };
            Vertex v4 = { x2 + pX, y2 + pY, currentColor.redF(), currentColor.greenF(), currentColor.blueF(), thick };

            vertices.append(v1);
            vertices.append(v2);
            vertices.append(v3);
            vertices.append(v4);
        }
    }
}

void Canvas::updateVertexBuffer() {
    if (vertices.isEmpty()) return;
    vBuffer.bind();
    vBuffer.allocate(vertices.data(), vertices.size() * sizeof(Vertex));
    vBuffer.release();
    std::cout << "VBO updated with " << vertices.size() << " vertices" << std::endl;
}

void Canvas::renderVertexBuffer()
{
    if (vertices.isEmpty()) return;

    if (!vBuffer.bind()) return;

    // Enable vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // Set vertex and color pointers
    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(0));
    glColorPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(2 * sizeof(float)));

    // Draw all quads
    for (int i = 0; i < vertices.size(); i += 4) {
        glDrawArrays(GL_QUADS, i, 4);
    }

    // Disable vertex arrays
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    vBuffer.release();
}

void Canvas::convertToOpenGLCoords(const QPointF& qtPoint, float& x, float& y) {
    x = static_cast<float>(qtPoint.x());
    y = static_cast<float>(qtPoint.y());
}

void Canvas::clearCanvas() {
    currentStroke.clear();
    strokeList.clear();
    vertices.clear();
    vboUpdateFlag = true;
    update();
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
        point.pressure = 0.5f;  // Starting pressure
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
        StrokePoint point;
        point.pos = event->pos();
        point.strokeTime = QTime::currentTime();

        // Calculate pressure based on speed
        if (!currentStroke.isEmpty()) {
            const StrokePoint& lastPoint = currentStroke.last();
            qint64 timeDelta = lastPoint.strokeTime.msecsTo(point.strokeTime);
            point.pressure = calculatePressure(point.pos, lastPoint.pos, timeDelta);
        }
        else {
            point.pressure = 0.5f;
        }

        point.thickness = minThickness + (maxThickness - minThickness) * point.pressure;
        currentStroke.append(point);
        update();
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton && drawing) {
        drawing = false;

        if (!currentStroke.isEmpty()) {
            addStrokeToVertexBuffer(currentStroke);  // Convert current stroke to quad vertices
            updateVertexBuffer();                    // Upload those to GPU
            if (currentStroke.size() > 1) {
                strokeList.append(currentStroke);
            }
            currentStroke.clear();
            update();                                // Repaint the canvas
        }
    }
}
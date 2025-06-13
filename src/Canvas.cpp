#pragma comment(lib, "opengl32.lib")
#include "Canvas.h"
#include "core/StrokeProcessor.h"
#include "core/BrushUtils.h"
#include <QShortcut>
#include <iostream>

// Canvas implementation TODO: Convert all of this to QOpenGLWindow
Canvas::Canvas(QWidget* parent) : QOpenGLWidget(parent), drawing(false), vboUpdateFlag(false)
{
    controller = new CanvasController();
    setMinimumSize(500, 500);
    QShortcut* u = new QShortcut(QKeySequence::Undo, this);
    connect(u, &QShortcut::activated, this, &Canvas::undo);

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
    const auto& stroke = controller->getCurrentStroke();
    if (!stroke.isEmpty() && stroke.size() > 1) {
        renderCurrentStroke();
    }

}
void Canvas::renderCurrentStroke() {
    QVector<Vertex> tempVertices;
    const auto& stroke = controller->getCurrentStroke();
    const auto& color = controller->getCurrentColor();

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
            convertToOpenGLCoords(currentPos, cx, cy);


            Vertex v1 = { cx + perpX, cy + perpY, color.redF(), color.greenF(), color.blueF(), thick };
            Vertex v2 = { cx - perpX, cy - perpY, color.redF(), color.greenF(), color.blueF(), thick };

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

// Fixed addStrokeToVertexBuffer
void Canvas::addStrokeToVertexBuffer(const QVector<StrokePoint>& stroke)
{
    auto newVertices = controller->getProcessor().generateVertices(stroke);
    vertices.append(newVertices);
    strokeVertexCounts.append(newVertices.size());
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

void Canvas::clearCanvas() {
    controller->clearCurrentStroke();
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
    if (controller->getCurrentStroke().isEmpty() && !strokeList.isEmpty()) {

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
    controller->setCurrentColor(color);
}

void Canvas::setBrushOptions(float minT, float maxT, float s) {
    minThickness = minT;
    maxThickness = maxT;
}

void Canvas::mousePressEvent(QMouseEvent* event)
{
    controller->onMousePress(event);
    timer.restart();
    update();
}


void Canvas::mouseMoveEvent(QMouseEvent* event)
{
    controller->onMouseMove(event);
    update();
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
    if ((Qt::LeftButton == event->button()) && controller->isDrawing()) {
        controller->onMouseLift(event);
        if (controller->getCurrentStroke().size() > 1) {
            addStrokeToVertexBuffer(controller->getCurrentStroke());
            updateVertexBuffer();
            strokeList.append(controller->getCurrentStroke());
        }
        controller->clearCurrentStroke();
        update();
    }
}
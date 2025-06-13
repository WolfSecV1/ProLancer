#pragma comment(lib, "opengl32.lib")
#include "Canvas.h"
#include "core/StrokeProcessor.h"
#include "core/math/mathUtils.h"
#include <QShortcut>
#include <iostream>

// Canvas implementation TODO: Convert all of this to QOpenGLWindow
Canvas::Canvas(QWidget* parent) : QOpenGLWidget(parent), vboUpdateFlag(false)
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
    // Canvas-wide OpenGL setup
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // Background color
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(3.0f);

    // Create Canvas's resources
    vBuffer.create();

    // Initialize renderer with Canvas resources
    controller->initializeRenderer(&vBuffer);

    if (!vBuffer.isCreated()) {
        vBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        vBuffer.create(); 
    }

    std::cout << "OpenGL Online" << std::endl;
}

void Canvas::paintGL()
{
    makeCurrent();

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
    if (!controller) return;

    const auto& stroke = controller->getCurrentStroke();
    const auto& color = controller->getCurrentColor();

    if (!stroke.isEmpty()) {
        auto& renderer = controller->getRenderer();
        renderer.renderStroke(stroke, color);  // Make sure renderer exists
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

void Canvas::renderVertexBuffer() {
    controller->getRenderer().renderVertexBuffer(vertices, strokeVertexCounts, vBuffer);
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
#pragma comment(lib, "opengl32.lib")
#include "Canvas.h"
#include <QShortcut>
#include <iostream>
#include <QTimer>

Canvas::Canvas(QWidget* parent) : QOpenGLWidget(parent), vboUpdateFlag(false)
{
    controller = std::make_unique<CanvasController>();
    setMinimumSize(500, 500);
    QShortcut* u = new QShortcut(QKeySequence::Undo, this);
    QShortcut* r = new QShortcut(QKeySequence::Redo, this);
    connect(u, &QShortcut::activated, this, &Canvas::undo);
    connect(r, &QShortcut::activated, this, &Canvas::redo);
    setAttribute(Qt::WA_TabletTracking);
    setMouseTracking(true);

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

#ifdef QT_DEBUG
    QString glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    QString glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    qDebug() << "OpenGL Version:" << glVersion;
    qDebug() << "OpenGL Renderer:" << glRenderer;
#endif
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

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        qWarning() << "Initial framebuffer incomplete! Status:" << status;
    }
}

void Canvas::paintGL()
{
    if (!context() || !context()->isValid()) {
        qWarning() << "Invalid OpenGL context!";
        return;
    }

    static bool renderBroken = false;
    if (renderBroken) return;

    try {
        makeCurrent();

#ifdef QT_DEBUG
        qDebug() << "paintGL() starting...";
#endif

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update VBO if needed
        if (vboUpdateFlag) {
            updateVertexBuffer();
            vboUpdateFlag = false;
        }

        // Render buffered strokes
        if (!vertices.isEmpty()) {
            renderVertexBuffer();
        }

        // Render live stroke
        const auto& stroke = controller->getCurrentStroke();
        if (!stroke.isEmpty() && stroke.size() > 1) {
            renderCurrentStroke();
        }

#ifdef QT_DEBUG
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            qDebug() << "GL Error:" << err;
        }
        qDebug() << "paintGL() finished";
#endif

    } catch (const std::exception& e) {
        renderBroken = true;
#ifdef QT_DEBUG
        qDebug() << "paintGL() exception:" << e.what();
#endif
    } catch (...) {
        renderBroken = true;
#ifdef QT_DEBUG
        qDebug() << "paintGL() unknown exception!";
#endif
    }
}


void Canvas::renderCurrentStroke() {
    if (!controller) return;

    const auto& stroke = controller->getCurrentStroke();
    if (stroke.isEmpty() || stroke.size() < 2) return;

    auto& renderer = controller->getRenderer();
    auto& processor = controller->getProcessor();

    QVector<StrokePoint> coloredStroke = stroke;
    QColor color = controller->getCurrentColor();

    for (auto& pt : coloredStroke) {
        pt.r = color.redF();
        pt.g = color.greenF();
        pt.b = color.blueF();
    }

    QVector<Vertex> verts = processor.generateVertices(coloredStroke);

    if (!verts.isEmpty()) {
        // Use a separate temporary buffer for the current stroke
        static QOpenGLBuffer tempBuffer(QOpenGLBuffer::VertexBuffer);
        if (!tempBuffer.isCreated()) {
            tempBuffer.create();
        }

        if (tempBuffer.bind()) {
            tempBuffer.allocate(verts.constData(), verts.size() * sizeof(Vertex));
            QVector<int> oneCount = { static_cast<int>(verts.size()) };
            renderer.renderVertexBuffer(verts, oneCount, tempBuffer);
            tempBuffer.release();
        }
    }
}


void Canvas::resizeGL(int width, int height) {
    glViewport(0, 0, width, height); // Set viewport

    glMatrixMode(GL_PROJECTION); // Switch to projection matrix stack
    glLoadIdentity(); // Reset Current Matrix to Identity matrix 
    glOrtho(0, width, height, 0, -1, 1); // Sets up Orthographic projection, Y is flipped because we are converting gl coords to Qt coords

    glMatrixMode(GL_MODELVIEW); // Matrix Ops affect model-voew matrix
    glLoadIdentity(); // Resert Current Matrix

}

// Fixed addStrokeToVertexBuffer
void Canvas::addStrokeToVertexBuffer(const QVector<StrokePoint>& stroke)
{
    int oldSize = vertices.size();
    controller->getManager().addStroke(stroke, controller->getProcessor(), vertices);

    if (vertices.size() > oldSize) {
        for (int i = oldSize; i < vertices.size(); ++i) {
            const Vertex& v = vertices[i];
            if (!std::isfinite(v.x) || !std::isfinite(v.y) ||
                std::abs(v.x) > 10000 || std::abs(v.y) > 10000) {
                std::cout << "INVALID VERTEX [" << i << "]: x=" << v.x << ", y=" << v.y << std::endl;
            }
        }
    }

    update();
}

void Canvas::updateVertexBuffer() {
    if (!vBuffer.isCreated()) {
        qWarning() << "VBuffer not created!";
        return;
    }
    controller->getRenderer().updateVertexBuffer(vBuffer, vertices);
}

void Canvas::renderVertexBuffer() {
    controller->getRenderer().renderVertexBuffer(vertices, controller->getManager().getStrokeVertexCounts(), vBuffer);
}

void Canvas::rebuildVertexBuffer() {
    vertices.clear();
    for (const QVector<StrokePoint>& stroke : controller->getManager().getStrokes()) {
        addStrokeToVertexBuffer(stroke);
    }
}

void Canvas::clearCanvas() {
    controller->clearCurrentStroke();
    controller->getManager().clear();
    vertices.clear();
    controller->getManager().clearStrokeVertexCounts();

    makeCurrent();
    glClear(GL_COLOR_BUFFER_BIT);

    // Clear the VBO
    controller->getRenderer().clearBuffer(vBuffer);

    update();
}

void Canvas::setColor(const QColor& color) {
    controller->setCurrentColor(color);
}

void Canvas::undo() {
    controller->getManager().undo(controller->getProcessor(), vertices);
    updateVertexBuffer();
    update();
}

void Canvas::redo() {
    controller->getManager().redo(controller->getProcessor(), vertices);
    updateVertexBuffer();
    update();
}

void Canvas::mousePressEvent(QMouseEvent* event)
{
#ifdef QT_DEBUG
    qDebug() << "Mouse Pressed";
#endif
    controller->getManager().setChangeSinceLastUndo(true);
    controller->getManager().clearRedoStack();
    controller->onMousePress(event);
    timer.restart();
    update();
}

void Canvas::tabletEvent(QTabletEvent* event)
{
#ifdef QT_DEBUG
    qDebug() << "tabletEvent ON";
#endif

    if (controller->tabletEvent(event)) {
        controller->getManager().setChangeSinceLastUndo(true);
        controller->getManager().clearRedoStack();
        timer.restart();
        update();
    }
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
            const auto& currentStroke = controller->getCurrentStroke();

            int maxPoints = (currentStroke.size() > 3) ? 3 : currentStroke.size();
            for (int i = 0; i < maxPoints; ++i) {
                const auto& pt = currentStroke[i];
            }


            // Check if any vertices are invalid
            for (int i = vertices.size() - 10; i < vertices.size(); ++i) {
                if (i >= 0) {
                    const auto& v = vertices[i];
                    if (!std::isfinite(v.x) || !std::isfinite(v.y) ||
                        std::abs(v.x) > 5000 || std::abs(v.y) > 5000) {
                        std::cout << "PROBLEMATIC VERTEX [" << i << "]: x=" << v.x << ", y=" << v.y << std::endl;
                    }
                }
            }

            addStrokeToVertexBuffer(controller->getCurrentStroke());

            // Check if any vertices are invalid
            for (int i = vertices.size() - 10; i < vertices.size(); ++i) {
                if (i >= 0) {
                    const auto& v = vertices[i];
                    if (!std::isfinite(v.x) || !std::isfinite(v.y) ||
                        std::abs(v.x) > 5000 || std::abs(v.y) > 5000) {
                        std::cout << "PROBLEMATIC VERTEX [" << i << "]: x=" << v.x << ", y=" << v.y << std::endl;
                    }
                }
            }

            vboUpdateFlag=true;
        }
        controller->clearCurrentStroke();
        update();
    }
}

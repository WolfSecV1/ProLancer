#ifndef CANVAS_H  
#define CANVAS_H  

#include <QMouseEvent>  
#include <QOpenGLBuffer>
#include <QOpenGLWidget>  
#include <QOpenGLFunctions>  
#include <QTime>
#include "../data/StrokePoint.h"
#include "core/CanvasController.h"
#include "data/Vertex.h"

class Canvas : public QOpenGLWidget, protected QOpenGLFunctions  
{  
    Q_OBJECT  

private:
    // Initializing required variables
    std::unique_ptr<CanvasController> controller;
    QElapsedTimer timer; // timer

    // VBO Stuff
    QOpenGLBuffer vBuffer;
    QVector<Vertex> vertices; // List of Vertex structs, append points to upload to vertexBuffer
    bool vboUpdateFlag; // Check if vertex data has changed

    void renderVertexBuffer();
    void rebuildVertexBuffer();
    void renderCurrentStroke();

public:  
    Canvas(QWidget* parent = nullptr); // Canvas class  
    ~Canvas();
    void addStrokeToVertexBuffer(const QVector<StrokePoint>& stroke);
    void updateVertexBuffer();
    void clearCanvas(); // Clear Canvas  
    void undo();
    void redo();
    void setColor(const QColor& color); // Sets pen color 
    void setBrushOptions(float min, float max, float s);

protected:  
    void initializeGL() override;  
    void paintGL() override;  
    void resizeGL(int width, int height) override;  

    void mousePressEvent(QMouseEvent* event) override;  
    void mouseMoveEvent(QMouseEvent* event) override;  
    void mouseReleaseEvent(QMouseEvent* event) override;  
};  

#endif

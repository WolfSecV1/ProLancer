#ifndef CANVAS_H  
#define CANVAS_H  

#include <QPainter>  
#include <QMouseEvent>  
#include <QPaintEvent>  
#include <QPoint>  
#include <QVector>  
#include <QColor>
#include <QOpenGLBuffer>
#include <QtOpenGLWidgets/QOpenGLWidget>  
#include <QOpenGLFunctions>  
#include <QTime>

struct StrokePoint {  
    QPointF pos;  // 2D float point, stores location on canvas, sub-pixel accuracy for smooth strokes
    float pressure;  // 0.0-1.0 pressure of pen input
    float thickness;  // thickness based on pressure and speed
    QTime strokeTime; // time when point was captured, calculate speed
};  

struct Vertex {
    float x, y;         // position
    float r, g, b;      // color 
    float thickness;    // thickness
};

class Canvas : public QOpenGLWidget, protected QOpenGLFunctions  
{  
    Q_OBJECT  

private:
    // Initializing required variables
    QVector<StrokePoint> currentStroke;  // Current stroke
    QVector<QVector<StrokePoint>> strokeList; // List of all strokes
    bool drawing; // Flag to see if currently drawing  
    QColor currentColor = Qt::black; // Set Color of ALL points.
    QElapsedTimer timer; // timer

    // VBO Stuff
    QOpenGLBuffer vBuffer; // Create buffer to store data on GPU instead of CPU
    QVector<Vertex> vertices; // List of Vertex structs, append points to upload to vertexBuffer
    bool vboUpdateFlag; // Check if vertex data has changed

    // Brush Options
    float minThickness = 1.0f;
    float maxThickness = 8.0f;
    float speedSensitivity = 0.5f;  // How much speed affects thickness

    void convertToOpenGLCoords(const QPointF& qtPoint, float& x, float& y); // Convert Qt to OpenGL coords  
    QVector<QPointF> interpolatePoints(const QPointF& p1, const QPointF& p2, int n_S = 3);
    float calculatePressure(const QPointF& posI, const QPointF& posF, qint64 deltaT);
    void addStrokeToVertexBuffer(const QVector<StrokePoint>& stroke);
    void updateVertexBuffer();
    void renderVertexBuffer();

public:  
    Canvas(QWidget* parent = nullptr); // Canvas class  
    ~Canvas();
    void clearCanvas(); // Clear Canvas  
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
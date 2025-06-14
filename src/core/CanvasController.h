#ifndef CANVASCONTROLLER_H
#define CANVASCONTROLLER_H

#include <QMouseEvent>
#include <QColor>
#include <QVector>
#include <QTime>
#include <memory>
#include <qopenglbuffer.h>
#include "../data/StrokePoint.h"  // your struct for points in a stroke
#include "../rendering/StrokeRenderer.h"
#include "StrokeProcessor.h"
#include "StrokeManager.h"

class CanvasController
{
public:

    CanvasController();

    // Call this when mouse press happens
    void onMousePress(QMouseEvent* event);
    void onMouseMove(QMouseEvent* event);
    void onMouseLift(QMouseEvent* event);

    // Getters + Setters
    const QVector<StrokePoint>& getCurrentStroke() const;
    void clearCurrentStroke();
    void appendToCurrentStroke(StrokePoint& point);
    bool isDrawing();
    void setDrawingToFalse();
    QColor getCurrentColor();
    void setCurrentColor(const QColor& color);

    void initializeRenderer(QOpenGLBuffer* buffer);

    StrokeProcessor& getProcessor() {
        return *strokeProcessor;  // Dereference the unique_ptr
    }

    StrokeRenderer& getRenderer() {
        return *strokeRenderer;  // Dereference the unique_ptr
    }

    StrokeManager& getManager() {
        return *strokeManager;  // Dereference the unique_ptr
    }

private:

    std::unique_ptr<StrokeProcessor> strokeProcessor;
    std::unique_ptr<StrokeRenderer> strokeRenderer;
    std::unique_ptr<StrokeManager> strokeManager;

    bool drawing = false;             // Are we currently drawing?

    QVector<StrokePoint> currentStroke;
    QColor currentColor = QColor(0, 0, 0);  // default black

    // You can add thickness limits here
    float minThickness = 1.0f;
    float maxThickness = 5.0f;
    float speedSensitivity = 0.75f;
};

#endif // CANVASCONTROLLER_H
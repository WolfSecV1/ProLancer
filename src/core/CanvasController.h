#ifndef CANVASCONTROLLER_H
#define CANVASCONTROLLER_H

#include <QMouseEvent>
#include <QColor>
#include <QVector>
#include <QTime>
#include "../data/StrokePoint.h"  // your struct for points in a stroke

class CanvasController
{
public:
    CanvasController();

    // Call this when mouse press happens
    void handleMousePress(QMouseEvent* event);

    // You can add getters to access stroke data, etc.
    const QVector<StrokePoint>& getCurrentStroke() const;
    void clearCurrentStroke();
    void appendToCurrentStroke(StrokePoint& point);
    bool isDrawing();
    void setDrawingToFalse();
    QColor getCurrentColor();
    void setCurrentColor(const QColor& color);

private:
    bool drawing = false;             // Are we currently drawing?
    QVector<StrokePoint> currentStroke;
    QColor currentColor = QColor(0, 0, 0);  // default black

    // You can add thickness limits here
    float minThickness = 1.0f;
    float maxThickness = 5.0f;
};

#endif // CANVASCONTROLLER_H
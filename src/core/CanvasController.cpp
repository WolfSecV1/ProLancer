#include "CanvasController.h"
#include <QDebug>

CanvasController::CanvasController() {}

void CanvasController::handleMousePress(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        qDebug() << "Mouse pressed at" << event->pos();

        drawing = true;
        currentStroke.clear();

        StrokePoint point;
        point.pos = event->pos();
        point.r = currentColor.redF();
        point.g = currentColor.greenF();
        point.b = currentColor.blueF();
        point.pressure = 0.2f;  // starting pressure
        point.thickness = minThickness + (maxThickness - minThickness) * point.pressure;
        point.strokeTime = QTime::currentTime();

        currentStroke.append(point);
    }
}

const QVector<StrokePoint>& CanvasController::getCurrentStroke() const {
    return currentStroke;
}

void CanvasController::clearCurrentStroke() {
    currentStroke.clear();
}

void CanvasController::appendToCurrentStroke(StrokePoint& point) {
    currentStroke.append(point);
}

bool CanvasController::isDrawing() {
    return drawing;
}

void CanvasController::setDrawingToFalse() {
    drawing = false;
}

QColor CanvasController::getCurrentColor() {
    return currentColor;
}

void CanvasController::setCurrentColor(const QColor& color) {
    currentColor = color;
}
#include "CanvasController.h"
#include "mathUtils.h"


CanvasController::CanvasController() {
    strokeProcessor = std::make_unique<StrokeProcessor>();
    strokeRenderer = std::make_unique<StrokeRenderer>();
    strokeManager = std::make_unique<StrokeManager>();
}

void CanvasController::onMousePress(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
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

void CanvasController::onMouseMove(QMouseEvent* event) {
    if (drawing && (Qt::LeftButton & event->buttons())) {
        QPointF newPos = event->pos();

        // Only add point if it's moved enough (reduces oversensitivity)
        if (!currentStroke.isEmpty()) {
            QPointF lastPos = currentStroke.last().pos;
            float dx = newPos.x() - lastPos.x();
            float dy = newPos.y() - lastPos.y();
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < 1.5f) return; // Skip if movement is too small
        }

        const auto& color = currentColor;
        StrokePoint point;
        point.pos = newPos;
        point.strokeTime = QTime::currentTime();
        point.r = color.redF();
        point.g = color.greenF();
        point.b = color.blueF();

        if (!currentStroke.isEmpty()) {
            const StrokePoint& lastPoint = currentStroke.last();
            qint64 timeDelta = lastPoint.strokeTime.msecsTo(point.strokeTime);
            point.pressure = calculatePressure(point.pos, lastPoint.pos, timeDelta, speedSensitivity);
            // Smooth pressure changes
            point.pressure = lastPoint.pressure * 0.25f + point.pressure * 0.75f;
        }
        else {
            point.pressure = 0.25f;
        }

        point.thickness = minThickness + (maxThickness - minThickness) * point.pressure;
        currentStroke.append(point);
    }
}

bool CanvasController::tabletEvent(QTabletEvent* event)
{
    const auto* device = event->pointingDevice();
    if (!device || device->type() != QInputDevice::DeviceType::Stylus)
        return false;

    if (!drawing || event->pressure() <= 0.01f)
        return false;

    if (event->type() != QEvent::TabletMove)
        return false;

    QPointF newPos = event->position();

    if (!currentStroke.isEmpty()) {
        QPointF lastPos = currentStroke.last().pos;
        float dx = newPos.x() - lastPos.x();
        float dy = newPos.y() - lastPos.y();
        float distance = std::sqrt(dx * dx + dy * dy);
        if (distance < 1.5f) return false;
    }

    // Add point
    StrokePoint point;
    point.pos = newPos;
    point.strokeTime = QTime::currentTime();

    const auto& color = currentColor;
    point.r = color.redF();
    point.g = color.greenF();
    point.b = color.blueF();

    point.pressure = std::max(static_cast<float>(event->pressure()), 0.01f);
    point.thickness = minThickness + (maxThickness - minThickness) * point.pressure;

    currentStroke.append(point);
    event->accept();
    return true; // Drawing occurred
}


void CanvasController::initializeRenderer(QOpenGLBuffer* buffer) {
    strokeRenderer->initialize(buffer);
}

void CanvasController::onMouseLift(QMouseEvent* event) {
        setDrawingToFalse();
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

#include "HSVColorPicker.h"
#include <QResizeEvent>
#include <QPainterPath>
#include <QtMath>
#include <algorithm>

HSVColorPicker::HSVColorPicker(QWidget* parent)
    : QWidget(parent)
    , currentColor(Qt::black)
    , currentHue(0.0f)
    , currentSaturation(0.0f)
    , currentValue(0.0f)
    , draggingWheel(false)
    , draggingTriangle(false)
{
    setCurrentColor(currentColor);
    setMinimumSize(200, 200);
    setMouseTracking(true);
    calculateGeometry();
}

void HSVColorPicker::setCurrentColor(const QColor& color) {
    currentColor = color;
    rgbToHsv(color, currentHue, currentSaturation, currentValue);
    update();
}

void HSVColorPicker::calculateGeometry() {
    int size = std::min(width(), height());
    center = QPointF(0.5f * width(), 0.5f * height());
    outerRadius = size * 0.35f;
    innerRadius = size * 0.275f;
    calculateTriangleVertcies();
}

void HSVColorPicker::calculateTriangleVertcies() {
    float scale = 0.9f;
    float r = innerRadius*scale;
    triangleVertices[0] = center + QPointF(0, -r);
    triangleVertices[1] = center + QPointF(-r * 0.866f, r * 0.5f);
    triangleVertices[2] = center + QPointF(r * 0.866f, r * 0.5f);
}

void HSVColorPicker::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    if (!painter.isActive()) {
        qDebug() << "Painter not active, skipping paintEvent";
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing);

    drawColorWheel(painter);
    drawHSVTriangle(painter);
    drawIndicators(painter);
}


void HSVColorPicker::drawColorWheel(QPainter& painter)
{
    // Draw color wheel
    for (int i = 0; i < 360; ++i) {
        float angle = qDegreesToRadians(i - 90);
        QColor wheelColor = hsvToRgb(i, 1.0f, 1.0f);

        QPen pen(wheelColor, 2.0f);
        painter.setPen(pen);

        QPointF innerPoint = center + QPointF(innerRadius * cos(angle), innerRadius * sin(angle));
        QPointF outerPoint = center + QPointF(outerRadius * cos(angle), outerRadius * sin(angle));

        painter.drawLine(innerPoint, outerPoint);
    }
}

float HSVColorPicker::getAngleFromPoint(const QPointF& point)
{
    QPointF diff = point - center;
    float angle = atan2(diff.y(), diff.x());
    angle = qRadiansToDegrees(angle) + 90.0f; // Adjust for top start
    if (angle < 0) angle += 360.0f;
    return angle;
}

QColor HSVColorPicker::hsvToRgb(float h, float s, float v){
    QColor color;
    color.setHsvF(h / 360.0f, s, v);
    return color;
}

void HSVColorPicker::rgbToHsv(const QColor& rgb, float& h, float& s, float& v)
{
    h = rgb.hsvHueF() * 360.0f;
    s = rgb.hsvSaturationF();
    v = rgb.valueF();
    if (h < 0) h = 0; // Handle undefined hue
}

void HSVColorPicker::drawHSVTriangle(QPainter& painter)
{
    // Create triangle path for clipping
    QPainterPath trianglePath;
    trianglePath.moveTo(triangleVertices[0]);
    trianglePath.lineTo(triangleVertices[1]);
    trianglePath.lineTo(triangleVertices[2]);
    trianglePath.closeSubpath();

    // Create a QImage for pixel-perfect rendering with proper alpha support
    QRect triangleBounds = trianglePath.boundingRect().toRect();
    triangleBounds = triangleBounds.intersected(rect()); // Clip to widget bounds

    if (triangleBounds.isEmpty()) return;

    // Use Format_ARGB32 for proper alpha channel support
    QImage triangleImage(triangleBounds.size(), QImage::Format_ARGB32);
    triangleImage.fill(Qt::transparent); // Fill with transparent background

    // Fill the image pixel by pixel
    for (int y = 0; y < triangleImage.height(); ++y) {
        for (int x = 0; x < triangleImage.width(); ++x) {
            QPointF pixelPoint = triangleBounds.topLeft() + QPointF(x, y);

            if (isPointInTriangle(pixelPoint)) {
                // Get saturation and value from triangle coordinates
                auto [saturation, value] = getSVFromTrianglePoint(pixelPoint);

                // Clamp values to valid range
                saturation = std::clamp(saturation, 0.0f, 1.0f);
                value = std::clamp(value, 0.0f, 1.0f);

                QColor pixelColor = QColor::fromHsvF(currentHue / 360.0f, saturation, value);
                triangleImage.setPixelColor(x, y, pixelColor);
            }
            // Pixels outside triangle remain transparent (no need to set them)
        }
    }

    // Draw the image
    painter.drawImage(triangleBounds.topLeft(), triangleImage);

    // Draw triangle outline
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(trianglePath);
}

void HSVColorPicker::drawIndicators(QPainter& painter)
{
    // Draw hue indicator on wheel
    float hueAngle = qDegreesToRadians(currentHue - 90.0f);
    QPointF huePoint = center + QPointF((innerRadius + outerRadius) * 0.5f * cos(hueAngle),
        (innerRadius + outerRadius) * 0.5f * sin(hueAngle));

    painter.setPen(QPen(Qt::white, 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(huePoint, 6, 6);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawEllipse(huePoint, 6, 6);

    // Draw SV indicator in triangle (static position)
    QPointF svPoint = getSVPointInTriangle(currentSaturation, currentValue);
    painter.setPen(QPen(Qt::white, 3));
    painter.drawEllipse(svPoint, 5, 5);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawEllipse(svPoint, 5, 5);
}

void HSVColorPicker::mousePressEvent(QMouseEvent* event)
{
    QPointF point = event->position();

    if (isPointInWheel(point) && !isPointInTriangle(point)) {
        draggingWheel = true;
        updateColorFromWheel(point);
    }
    else if (isPointInTriangle(point)) {
        draggingTriangle = true;
        updateColorFromTriangle(point);
    }
}

void HSVColorPicker::mouseMoveEvent(QMouseEvent* event)
{
    QPointF point = event->position();

    if (draggingWheel && isPointInWheel(point)) {
        updateColorFromWheel(point);
    }
    else if (draggingTriangle && isPointInTriangle(point)) {
        updateColorFromTriangle(point);
    }
}

void HSVColorPicker::mouseReleaseEvent(QMouseEvent* event)
{
    draggingWheel = false;
    draggingTriangle = false;
}

QPointF HSVColorPicker::getSVPointInTriangle(float saturation, float value)
{
    // This should be the exact inverse of getSVFromTrianglePoint
    // From getSVFromTrianglePoint:
    // saturation = u (pure hue weight)
    // value = w + u (white weight + pure hue weight)
    // So: w = value - saturation, u = saturation, v = 1 - u - w

    QPointF white = triangleVertices[0];     // Top
    QPointF black = triangleVertices[1];     // Bottom-left
    QPointF pure = triangleVertices[2];      // Bottom-right

    float u = saturation;                           // Pure hue weight
    float w = value - saturation;                   // White weight
    float v = 1.0f - u - w;                        // Black weight = 1 - saturation - (value - saturation) = 1 - value

    return white * w + black * v + pure * u;
}

QPair<float, float> HSVColorPicker::getSVFromTrianglePoint(const QPointF& point)
{
    // Convert triangle point back to S,V coordinates using barycentric coordinates
    QPointF v0 = triangleVertices[2] - triangleVertices[0]; // pure - white
    QPointF v1 = triangleVertices[1] - triangleVertices[0]; // black - white
    QPointF v2 = point - triangleVertices[0];               // point - white

    float dot00 = QPointF::dotProduct(v0, v0);
    float dot01 = QPointF::dotProduct(v0, v1);
    float dot02 = QPointF::dotProduct(v0, v2);
    float dot11 = QPointF::dotProduct(v1, v1);
    float dot12 = QPointF::dotProduct(v1, v2);

    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom; // pure hue weight
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom; // black weight
    float w = 1.0f - u - v;                               // white weight

    // Convert barycentric to S,V
    float saturation = std::clamp(u, 0.0f, 1.0f);
    float value = std::clamp(w + u, 0.0f, 1.0f); // white + pure hue components

    return qMakePair(saturation, value);
}

void HSVColorPicker::resizeEvent(QResizeEvent* event)
{
    calculateGeometry();
    QWidget::resizeEvent(event);
}

void HSVColorPicker::updateColorFromWheel(const QPointF& point)
{
    currentHue = getAngleFromPoint(point);
    // Triangle stays static - no need to update vertices
    currentColor = hsvToRgb(currentHue, currentSaturation, currentValue);
    emit colorChanged(currentColor);
    update();
}

void HSVColorPicker::updateColorFromTriangle(const QPointF& point)
{
    auto [s, v] = getSVFromTrianglePoint(point);
    currentSaturation = std::clamp(s, 0.0f, 1.0f);
    currentValue = std::clamp(v, 0.0f, 1.0f);
    currentColor = hsvToRgb(currentHue, currentSaturation, currentValue);
    emit colorChanged(currentColor);
    update();
}

bool HSVColorPicker::isPointInWheel(const QPointF& point)
{
    float distance = sqrt(pow(point.x() - center.x(), 2) + pow(point.y() - center.y(), 2));
    return distance >= innerRadius && distance <= outerRadius;
}

bool HSVColorPicker::isPointInTriangle(const QPointF& point) {
  
    QPointF v0 = triangleVertices[2] - triangleVertices[0];
    QPointF v1 = triangleVertices[1] - triangleVertices[0];
    QPointF v2 = point - triangleVertices[0];

    float dot00 = QPointF::dotProduct(v0, v0);  
    float dot01 = QPointF::dotProduct(v0, v1);  
    float dot02 = QPointF::dotProduct(v0, v2);  
    float dot11 = QPointF::dotProduct(v1, v1);  
    float dot12 = QPointF::dotProduct(v1, v2);

    float invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    return (u >= 0) && (v >= 0) && (u + v <= 1);

}

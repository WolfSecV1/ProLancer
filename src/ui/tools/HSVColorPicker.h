#ifndef HSVCOLORPICKER_H
#define HSVCOLORPICKER_H

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QColor>
#include <QPainter>
#include <QPointF>
#include <cmath>

class HSVColorPicker : public QWidget
{
    Q_OBJECT

public:
    explicit HSVColorPicker(QWidget* parent = nullptr);
    QColor getCurrentColor() const { return currentColor; }
    void setCurrentColor(const QColor& color);

signals:
    void colorChanged(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    // Color wheel properties
    QPointF center;
    float outerRadius;
    float innerRadius;

    // Triangle properties
    QPointF triangleVertices[3];

    // Current color state
    QColor currentColor;
    float currentHue;        // 0-360
    float currentSaturation; // 0-1
    float currentValue;      // 0-1

    // Interaction state
    bool draggingWheel;
    bool draggingTriangle;

    // Helper methods
    void calculateGeometry();
    void calculateTriangleVertcies();
    void drawColorWheel(QPainter& painter);
    void drawHSVTriangle(QPainter& painter);
    void drawIndicators(QPainter& painter);

    float getAngleFromPoint(const QPointF& point);
    QPointF getPointOnWheel(float angle);
    QPointF getSVPointInTriangle(float saturation, float value);
    QPair<float, float> getSVFromTrianglePoint(const QPointF& point);
    bool isPointInTriangle(const QPointF& point);
    bool isPointInWheel(const QPointF& point);

    void updateColorFromWheel(const QPointF& point);
    void updateColorFromTriangle(const QPointF& point);

    QColor hsvToRgb(float h, float s, float v);
    void rgbToHsv(const QColor& rgb, float& h, float& s, float& v);
};

#endif // HSVCOLORPICKER_H

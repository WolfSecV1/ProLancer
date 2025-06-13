#include "mathUtils.h"

float calculatePressure(const QPointF& posF, const QPointF& posI, qint64 deltaT, float speedSense) {
    if (deltaT <= 0) {
        return 0.5f;
    }
    float dx = posF.x() - posI.x();
    float dy = posF.y() - posI.y();
    float dist = std::sqrt(dx * dx + dy * dy);
    float sec = deltaT / 1000.0f;
    float speed = dist / sec;
    float maxSpeed = 1000.0f;
    float pressure = 1.0f - (speed / maxSpeed) * speedSense;
    if (pressure < 0.1f) { pressure = 0.1f; }
    if (pressure > 1.0f) { pressure = 1.0f; }
    return pressure;
}

void convertToOpenGLCoords(const QPointF& qtPoint, float& x, float& y) {
    x = static_cast<float>(qtPoint.x());
    y = static_cast<float>(qtPoint.y());
}


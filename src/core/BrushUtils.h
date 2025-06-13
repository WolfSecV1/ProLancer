#ifndef BRUSHUTILS_H
#define BRUSHUTILS_H

#include <QPoint>

float calculatePressure(const QPointF& posF, const QPointF& posI, qint64 deltaT, float speedSense);
void convertToOpenGLCoords(const QPointF& qtPoint, float& x, float& y);

#endif


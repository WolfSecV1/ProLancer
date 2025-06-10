#ifndef STROKEPOINT_H
#define STROKEPOINT_H

#include <qpoint.h>
#include <QTime>

struct StrokePoint {
    QPointF pos;  // 2D float point, stores location on canvas, sub-pixel accuracy for smooth strokes
    float pressure;  // 0.0-1.0 pressure of pen input
    float thickness;  // thickness based on pressure and speed
    QTime strokeTime; // time when point was captured, calculate speed
    float r, g, b; //color
};

#endif // !STROKEPOINT_H
#include "StrokeProcessor.h"
#include "math/mathUtils.h"

StrokeProcessor::StrokeProcessor() {}

QVector<QPointF> StrokeProcessor::interpolatePoints(const QPointF& p1, const QPointF& p2, int segments) {
    QVector<QPointF> result; //Create empty stroke
    result.append(p1); //start with first point
    for (int i = 0; i <= segments; ++i) { //loop for # of segments
        float t = static_cast<float>(i) / segments; // t = i/n

        //Apply LERP formula to our new point
        QPointF interpolated(
            p1.x() * (1 - t) + p2.x() * t,
            p1.y() * (1 - t) + p2.y() * t
        );
        result.append(interpolated); //Append it
    }
    result.append(p2); // Last but not lease attach the last point
    return result; //Return the stroke
}

QVector<Vertex> StrokeProcessor::generateVertices(const QVector<StrokePoint>& stroke) {

    QVector<Vertex> vertices;

    if (stroke.size() < 2) return vertices;

    for (int i = 0; i < stroke.size() - 1; ++i) {
        const StrokePoint& p1 = stroke[i];
        const StrokePoint& p2 = stroke[i + 1];

        // Reduce interpolation - only interpolate if points are far apart
        float dx = p2.pos.x() - p1.pos.x();
        float dy = p2.pos.y() - p1.pos.y();
        float distance = std::sqrt(dx * dx + dy * dy);

        QVector<QPointF> points;
        if (distance > 5.0f) {
            // Only interpolate if points are far apart
            points = interpolatePoints(p1.pos, p2.pos, 3); // Reduce interpolation
        }
        else {
            points.append(p1.pos);
            points.append(p2.pos);
        }

        for (int j = 0; j < points.size() - 1; ++j) {
            QPointF currentPos = points[j];
            QPointF nextPos = points[j + 1];

            // Calculate direction
            float dirX = nextPos.x() - currentPos.x();
            float dirY = nextPos.y() - currentPos.y();
            float len = std::sqrt(dirX * dirX + dirY * dirY);

            if (len < 0.1f) continue;

            dirX /= len;
            dirY /= len;

            // Interpolate thickness
            float t = static_cast<float>(j) / (points.size() - 1);
            float thick = p1.thickness * (1.0f - t) + p2.thickness * t;
            thick = std::min<float>(thick, 4.0f) * 0.5f; // Limit and reduce thickness
            // Perpendicular offset
            float perpX = -dirY * thick;
            float perpY = dirX * thick;

            float cx, cy;
            convertToOpenGLCoords(currentPos, cx, cy);

            Vertex v1 = { cx + perpX, cy + perpY, p1.r, p1.g, p1.b, thick };
            Vertex v2 = { cx - perpX, cy - perpY, p2.r, p2.g, p2.b, thick };

            vertices.append(v1);
            vertices.append(v2);
        }
    }

    return vertices;
}
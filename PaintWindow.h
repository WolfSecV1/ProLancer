// PaintWindow.h
#ifndef PAINTWINDOW_H
#define PAINTWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPoint>
#include <QVector>

class Canvas : public QWidget
{
    Q_OBJECT

private:
    QVector<QPoint> points;  // Store all drawing points
    bool drawing;
    QColor currentColor = Qt::black;

public:
    Canvas(QWidget* parent = nullptr);
    void clearCanvas();
    void setColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
};

class PaintWindow : public QMainWindow
{
    Q_OBJECT

public:
    PaintWindow(QWidget* parent = nullptr);

private:
    Canvas* canvas;
};

#endif // PAINTWINDOW_H
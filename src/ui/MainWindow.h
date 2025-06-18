#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include "tools/HSVColorPicker.h"

class Canvas;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onColorChanged(const QColor& color);

private:
    Canvas* canvas;
    HSVColorPicker* colorPicker;
    QWidget* leftSidebar;
    QTabWidget* sidebarTabs;
    QSplitter* mainSplitter;

    QString loadVersion();
    void setupUI();
    void setupLeftSidebar();
};

#endif // MAINWINDOW_H

// main.cpp
#include <QApplication>
#include "PaintWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    PaintWindow window;
    window.show();

    return app.exec();
}
// PaintWindow.cpp
#include "MainWindow.h"
#include "Canvas.h"
#include "../fileReader.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <iostream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    QString lancerVersion;

    try {
        lancerVersion = loadFileAsQString("version.txt").trimmed();
    }
    catch (const std::exception& e) {
        lancerVersion = "Unknown Version";
        qDebug() << "Error loading version:" << e.what();
    }

    setWindowTitle("Lancer: " + lancerVersion);
    setMinimumSize(600, 600);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout* toolLayout = new QHBoxLayout();

    QPushButton* clearButton = new QPushButton("Clear Canvas");
    QPushButton* colorButton = new QPushButton("Choose Color");
    QPushButton* undoButton = new QPushButton("Undo");

    QWidget* colorPanel = new QWidget(nullptr, Qt::Popup);  // Not a child of layout
    QGridLayout* cLayout = new QGridLayout(colorPanel);
    colorPanel->setLayout(cLayout);

    QVector<QColor> swatchColors = {
    Qt::black, Qt::red, Qt::green, Qt::blue, Qt::yellow,
    QColor("#FFA500"), // orange
    QColor("#800080"), // purple
    QColor("#00FFFF")  // cyan
    };

    
    for (int i = 0; i < swatchColors.size(); ++i) {  
        QPushButton* cButton = new QPushButton(colorPanel);  
        cButton->setFixedSize(20, 20);  
        cButton->setStyleSheet(QString("background-color: %1; border: 1px solid #555;")  
            .arg(swatchColors[i].name()));  
        connect(cButton, &QPushButton::clicked, [this, swatchColors, i, colorPanel]() {  
            canvas->setColor(swatchColors[i]);  
            colorPanel->setVisible(false);  
        });  

        int row = i / 4;  
        int col = i % 4;  
        cLayout->addWidget(cButton, row, col);  
    }


    colorPanel->setLayout(cLayout);
    colorPanel->setVisible(false);

    toolLayout->addWidget(clearButton);
    toolLayout->addWidget(colorButton);
    toolLayout->addWidget(undoButton);
    toolLayout->addStretch(); // Push buttons to left

    // Create canvas
    canvas = new Canvas(this);

    // Add to layout
    mainLayout->addLayout(toolLayout);
    mainLayout->addWidget(canvas);

    connect(clearButton, &QPushButton::clicked, [this]() {
        canvas->clearCanvas();
        std::cout << "Clear button clicked!" << std::endl;
        });

    connect(colorButton, &QPushButton::clicked, [colorPanel, colorButton]() {
        // TODO: Color picker functionality  
        QPoint globalPos = colorButton->mapToGlobal(QPoint(0, colorButton->height()));
        colorPanel->move(globalPos);
        if (colorPanel->isVisible())
            colorPanel->setVisible(false);
        else
            colorPanel->setVisible(true);
        std::cout << "Color button clicked!" << std::endl;
        });

    connect(undoButton, &QPushButton::clicked, [this]() {
        canvas->undo();
        });
}   
        

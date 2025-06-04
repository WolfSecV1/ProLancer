// PaintWindow.cpp
#include "PaintWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <iostream>

// Canvas implementation
Canvas::Canvas(QWidget* parent) : QWidget(parent), drawing(false)
{
    setMinimumSize(800, 600);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);
}

void Canvas::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Set pen properties
    QPen pen(currentColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);

    // Draw all the points as connected lines
    if (points.size() > 1) {
        for (int i = 1; i < points.size(); ++i) {
            painter.drawLine(points[i - 1], points[i]);
        }
    }

    std::cout << "Paint event - drawing " << points.size() << " points" << std::endl;
}

void Canvas::clearCanvas() {
    points.clear();
    update();
}

void Canvas::setColor(const QColor& color) {
    currentColor = color;
}

void Canvas::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        drawing = true;
        points.append(event->pos());
        update(); // Trigger repaint
    }
}

void Canvas::mouseMoveEvent(QMouseEvent* event)
{
    if (drawing && (event->buttons() & Qt::LeftButton)) {
        points.append(event->pos());
        update(); // Trigger repaint
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        drawing = false;
    }
}

PaintWindow::PaintWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("ProLancer: v0.1.1");
    setMinimumSize(900, 700);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Create toolbar
    QHBoxLayout* toolLayout = new QHBoxLayout();

    QPushButton* clearButton = new QPushButton("Clear Canvas");
    QPushButton* colorButton = new QPushButton("Choose Color");

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
}


/*
LEVEL 2 CHALLENGES:
1. Implement the Clear button - clear all points and repaint
2. Add color picker functionality - change drawing color
3. Add brush size controls - make lines thicker/thinner
4. Add a status bar showing current mouse coordinates
5. BONUS: Add keyboard shortcuts (Ctrl+Z for undo)

CONCEPTS YOU'RE LEARNING:
- Qt application structure (QApplication, QMainWindow, QWidget)
- Custom widgets and event handling
- QPainter for real graphics rendering
- Signals and slots (Qt's event system)
- Layout management
- Mouse event coordination

NEXT LEVEL PREVIEW:
Level 3 will add smooth brush strokes, pressure sensitivity prep,
and proper canvas management with layers!

Ready to tackle the challenges? Start with #1 (Clear button)!
*/
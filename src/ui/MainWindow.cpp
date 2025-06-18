// PaintWindow.cpp
#include "MainWindow.h"
#include "Canvas.h"
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <iostream>
#include <QLabel>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    QString lancerVersion = loadVersion();
    setWindowTitle("Lancer: v" + lancerVersion);
    setMinimumSize(800, 800);
    setupUI();
}

QString MainWindow::loadVersion()
{
    // Try to load from Qt resources first (for deployment)
    QFile resourceFile(":/assets/version.txt");
    if (resourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString version = QString::fromUtf8(resourceFile.readAll()).trimmed();
        if (!version.isEmpty()) {
            qDebug() << "Loaded version from resources:" << version;
            return version;
        }
    }

    // Try to load from build directory (for development)
    QFile buildFile(QCoreApplication::applicationDirPath() + "/version.txt");
    if (buildFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString version = QString::fromUtf8(buildFile.readAll()).trimmed();
        if (!version.isEmpty()) {
            qDebug() << "Loaded version from build directory:" << version;
            return version;
        }
    }

    // Try to load from source directory (fallback for development)
    QFile sourceFile("assets/version.txt");
    if (sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString version = QString::fromUtf8(sourceFile.readAll()).trimmed();
        if (!version.isEmpty()) {
            qDebug() << "Loaded version from source directory:" << version;
            return version;
        }
    }

    qDebug() << "Could not load version from any location";
    return "Unknown Version";
}

void MainWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Create main layout for the central widget
    QHBoxLayout* centralLayout = new QHBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    // Create main horizontal splitter
    mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);

    setupLeftSidebar();

    QWidget* mainContent = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(mainContent);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    QHBoxLayout* toolLayout = new QHBoxLayout();

    QPushButton* clearButton = new QPushButton("Clear Canvas");
    QPushButton* undoButton = new QPushButton("Undo");

    toolLayout->addWidget(clearButton);
    toolLayout->addWidget(undoButton);
    toolLayout->addStretch(); // Push buttons to left

    // Create canvas
    canvas = new Canvas(this);

    // Add to layout
    mainLayout->addLayout(toolLayout);
    mainLayout->addWidget(canvas, 1); // Give canvas stretch factor of 1

    mainSplitter->addWidget(leftSidebar);
    mainSplitter->addWidget(mainContent);

    // Set splitter sizes and policies
    mainSplitter->setStretchFactor(0, 0); // Sidebar doesn't stretch
    mainSplitter->setStretchFactor(1, 1); // Main content stretches
    mainSplitter->setSizes({200, 800}); // Initial sizes

    mainSplitter->setMinimumWidth(200);
    mainSplitter->setMaximumWidth(400);
    mainSplitter->setHandleWidth(6);
    mainSplitter->setChildrenCollapsible(false);

    // Add splitter to central layout
    centralLayout->addWidget(mainSplitter);

    connect(clearButton, &QPushButton::clicked, [this]() {
        canvas->clearCanvas();
        std::cout << "Clear button clicked!" << std::endl;
    });

    connect(undoButton, &QPushButton::clicked, [this]() {
        canvas->undo();
    });
}

void MainWindow::setupLeftSidebar()
{
    leftSidebar = new QWidget();


    QVBoxLayout* sidebarLayout = new QVBoxLayout(leftSidebar);
    sidebarLayout->setContentsMargins(5, 5, 0, 5);

    // Create tab widget for sidebar
    sidebarTabs = new QTabWidget();
    sidebarTabs->setTabPosition(QTabWidget::North);
    sidebarTabs->setStyleSheet(
        "QTabWidget::pane {"
        "    border: 1px solid #ddd;"
        "    background-color: grey;"
        "}"
        "QTabBar::tab {"
        "    background-color: grey;"
        "    padding: 7px 50px;"
        "    margin-right: 2px;"
        "    border-top-left-radius: 4px;"
        "    border-top-right-radius: 4px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: black;"
        "}"
        "QTabBar::tab:hover {"
        "    background-color: grey;"
        "}"
        );

    // Create color picker tab
    QWidget* colorTab = new QWidget();
    QVBoxLayout* colorTabLayout = new QVBoxLayout(colorTab);
    colorTabLayout->setContentsMargins(5, 5, 5, 5);

    // Add title
    QLabel* colorTitle = new QLabel("Color Picker");
    colorTitle->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #333; }");
    colorTitle->setAlignment(Qt::AlignCenter);
    colorTabLayout->addWidget(colorTitle);

    // Add HSV color picker
    colorPicker = new HSVColorPicker();
    colorPicker->setMinimumSize(200, 200);
    colorTabLayout->addWidget(colorPicker);

    // Add current color display
    QLabel* currentColorLabel = new QLabel("Current Color:");
    currentColorLabel->setStyleSheet("QLabel { font-weight: bold; margin-top: 10px; color: #333; }");
    colorTabLayout->addWidget(currentColorLabel);

    QWidget* colorDisplay = new QWidget();
    colorDisplay->setFixedHeight(30);
    colorDisplay->setStyleSheet("QWidget { background-color: black; border: 2px solid #333; border-radius: 4px; }");
    colorTabLayout->addWidget(colorDisplay);

    colorTabLayout->addStretch(); // Push content to top

    // Connect color picker signal
    connect(colorPicker, &HSVColorPicker::colorChanged, this, &MainWindow::onColorChanged);
    connect(colorPicker, &HSVColorPicker::colorChanged, [colorDisplay](const QColor& color) {
        colorDisplay->setStyleSheet(QString(
                                        "QWidget { background-color: %1; border: 2px solid #333; border-radius: 4px; }"
                                        ).arg(color.name()));
    });

    // Add color tab to tab widget
    sidebarTabs->addTab(colorTab, "Colors");

    // You can add more tabs here in the future
    // sidebarTabs->addTab(brushTab, "Brushes");
    // sidebarTabs->addTab(layersTab, "Layers");

    sidebarLayout->addWidget(sidebarTabs);
}

void MainWindow::onColorChanged(const QColor& color)
{
    canvas->setColor(color);
    std::cout << "Color changed to: " << color.name().toStdString() << std::endl;
}

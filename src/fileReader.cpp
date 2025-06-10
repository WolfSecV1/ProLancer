#include <QFile>
#include <QTextStream>
#include <QString>

QString loadFileAsQString(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error("Failed to open file: " + filePath.toStdString());

    QTextStream in(&file);
    return in.readAll();
}
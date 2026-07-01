#include "SettingsDialog.hpp"
#include <QVBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QSpinBox>
#include <QLabel>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDir>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), currentBgColor(240, 240, 240) {
    setWindowTitle("Настройки интерфейса");

    QVBoxLayout* layout = new QVBoxLayout(this);

    // 1. Выбор цвета фона
    layout->addWidget(new QLabel("Цвет фона:"));
    colorBtn = new QPushButton("Выбрать цвет", this);
    connect(colorBtn, &QPushButton::clicked, this, &SettingsDialog::chooseBgColor);
    layout->addWidget(colorBtn);

    // 2. Выбор размера шрифта
    layout->addWidget(new QLabel("Размер шрифта:"));
    fontSizeSpin = new QSpinBox(this);
    fontSizeSpin->setRange(8, 24);
    fontSizeSpin->setValue(9); // Значение по умолчанию
    layout->addWidget(fontSizeSpin);

    // 3. Кнопка сохранения
    QPushButton* saveBtn = new QPushButton("Сохранить и применить", this);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::saveToJson);
    layout->addWidget(saveBtn);
}

void SettingsDialog::chooseBgColor() {
    QColor color = QColorDialog::getColor(currentBgColor, this, "Выберите цвет фона");
    if (color.isValid()) {
        currentBgColor = color;
        // Можно тут же поменять цвет самой кнопки для наглядности
        colorBtn->setStyleSheet(QString("background-color: %1").arg(color.name()));
    }
}

void SettingsDialog::saveToJson() {
    QString configPath = QCoreApplication::applicationDirPath() + QDir::separator() + "config.json";

    // Сначала читаем текущий файл, чтобы не затереть предметы
    QFile file(configPath);
    QJsonDocument doc;
    if (file.open(QIODevice::ReadOnly)) {
        doc = QJsonDocument::fromJson(file.readAll());
        file.close();
    }

    QJsonObject root = doc.object();

    // Создаем или обновляем объект с настройками UI
    QJsonObject uiSettings;
    uiSettings["bg_color"] = currentBgColor.name(); // Сохранит в виде "#F0F0F0"
    uiSettings["font_size"] = fontSizeSpin->value();

    root["ui_settings"] = uiSettings;
    doc.setObject(root);

    // Записываем обратно в файл
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }

    emit settingsApplied(); // Сигналим MainWindow, что пора обновляться
    accept(); // Закрываем диалог
}
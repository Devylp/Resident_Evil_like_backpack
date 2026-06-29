#include "MainWindow.hpp"
#include "BackpackLogick.hpp"
#include "BackpackView.hpp"
#include "Item.hpp"

#include <QToolBar>
#include <QPushButton>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDir>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Backpack Inventory System");

    // 1. Инициализируем логику базовыми значениями (на случай, если JSON не загрузится)
    mainLogic = new BackpackLogick(6, 5, 100);
    beltLogic = new BackpackLogick(4, 1, 15);
    handsLogic = new BackpackLogick(2, 2, 20);

    // 2. Загружаем данные из JSON
    QString exePath = QCoreApplication::applicationDirPath();
    QString configPath = exePath + QDir::separator() + "config.json";
    loadFromJson(configPath);

    // 3. Создаем графическое отображение ТОЛЬКО для главного рюкзака (пока что)
    mainView = new BackpackView(mainLogic, this);
    setCentralWidget(mainView);
    resize(mainView->sizeHint());

    // 4. Панель инструментов
    QToolBar* toolbar = addToolBar("Tools");
    QPushButton* sortBtn = new QPushButton("Auto Sort Main", this);
    toolbar->addWidget(sortBtn);

    connect(sortBtn, &QPushButton::clicked, this, &MainWindow::onAutoSort);
}

void MainWindow::loadFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open JSON file:" << filePath << "Using hardcoded fallback.";

        // Фолбэк, если JSON не найден (как было раньше)
        Item sword{ 1, "", "Sword", 5, 2, 1, -1, -1, true };
        mainLogic->AddItem(sword, 0, 0);
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();

    // Парсим параметры рюкзаков (опционально, можно расширить)
    if (root.contains("backpacks")) {
        QJsonObject bps = root["backpacks"].toObject();
        // Здесь мы могли бы пересоздать mainLogic с новыми размерами из JSON
        // Но для экономии времени оставим пока логику создания в конструкторе.
    }

    // Парсим предметы
    if (root.contains("items")) {
        QJsonArray itemsArray = root["items"].toArray();
        int startX = 0; // Временная координата X для расстановки

        for (const auto& value : itemsArray) {
            QJsonObject obj = value.toObject();

            Item item;
            item.ID = obj["id"].toInt();
            item.NameItem = obj["name"].toString().toStdString();
            item.weight = obj["weight"].toInt();
            item.wth = obj["width"].toInt();
            item.len = obj["length"].toInt();
            item.texturePath = obj["texture"].toString().toStdString();
            item.rotatable = obj["rotatable"].toBool();

            // Добавляем предметы в главный рюкзак друг за другом
            if (mainLogic->CanPlaceItem(item, startX, 0)) {
                mainLogic->AddItem(item, startX, 0);
                startX += item.wth;
            }
        }
    }
}

void MainWindow::onAutoSort()
{
    auto notPlaced = mainLogic->AutoSortBackpack();
    mainView->UpdateView();

    if (!notPlaced.empty()) {
        QMessageBox::warning(this, "Сортировка", "Не все предметы поместились в рюкзак!");
    }
}
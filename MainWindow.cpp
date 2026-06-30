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
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Backpack Inventory System");

    // 1. Инициализируем логику СТРОГО под размеры (Строки/Length, Колонки/Width, Вес)
    mainLogic = new BackpackLogick(6, 5, 100);  // 6 строк, 5 колонок
    beltLogic = new BackpackLogick(1, 4, 15);   // 1 строка, 4 колонки
    handsLogic = new BackpackLogick(2, 2, 20);  // 2 строки, 2 колонки

    // 2. Загружаем JSON
    QString exePath = QCoreApplication::applicationDirPath();
    QString configPath = exePath + QDir::separator() + "config.json";
    loadFromJson(configPath);

    // 3. Создаем ОДНО единое представление
    unifiedView = new BackpackView(mainLogic, beltLogic, handsLogic, this);

    // 4. Компонуем окно
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->addWidget(unifiedView);

    setCentralWidget(centralWidget);
    centralWidget->adjustSize();
    resize(centralWidget->sizeHint());

    // 5. Панель инструментов
    QToolBar* toolbar = addToolBar("Tools");
    QPushButton* sortBtn = new QPushButton("Auto Sort Main", this);
    toolbar->addWidget(sortBtn);

    connect(sortBtn, &QPushButton::clicked, this, &MainWindow::onAutoSort);
}

void MainWindow::loadFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open JSON file:" << filePath;
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();

    auto parseContainer = [](const QJsonArray& itemsArray, BackpackLogick* targetLogic) {
        int startX = 0;
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

            if (targetLogic->CanPlaceItem(item, startX, 0)) {
                targetLogic->AddItem(item, startX, 0);
                startX += item.wth;
            }
        }
        };

    if (root.contains("backpack_items")) parseContainer(root["backpack_items"].toArray(), mainLogic);
    if (root.contains("belt_items"))     parseContainer(root["belt_items"].toArray(), beltLogic);
    if (root.contains("hands_items"))    parseContainer(root["hands_items"].toArray(), handsLogic);
}

void MainWindow::onAutoSort()
{
    auto notPlaced = mainLogic->AutoSortBackpack();
    unifiedView->UpdateView();

    if (!notPlaced.empty()) {
        QMessageBox::warning(this, "Сортировка", "Не все предметы поместились в рюкзак!");
    }
}

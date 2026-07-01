#include "MainWindow.hpp"
#include "BackpackLogick.hpp"
#include "BackpackView.hpp"
#include "Item.hpp"
#include "SettingsDialog.hpp"

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
#include <QFontDatabase>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Backpack Inventory System");

    QString fontPath = QCoreApplication::applicationDirPath() + "/fonts/itc_medium.otf";
    int fontId = QFontDatabase::addApplicationFont(fontPath);

    if (fontId != -1) {
        loadedFontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    }
    else {
        qWarning() << "Не удалось загрузить шрифт! Используем Segoe UI.";
    }

    // 1. Инициализируем логику
    mainLogic = new BackpackLogick(6, 5, 100);
    beltLogic = new BackpackLogick(1, 4, 15);
    handsLogic = new BackpackLogick(2, 2, 20);
    storageLog = new BackpackLogick(8, 3, 9999);

    // 2. Загружаем JSON (предметы)
    QString exePath = QCoreApplication::applicationDirPath();
    QString configPath = exePath + QDir::separator() + "config.json";
    loadFromJson(configPath);

    // 3. Создаем ОДНО единое представление
    unifiedView = new BackpackView(mainLogic, beltLogic, handsLogic, storageLog, this);

    int startFontSize = 9;
    QColor startBgColor(240, 240, 240); // Дефолт, если в JSON что-то пойдет не так

    QFile file(configPath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject root = doc.object();
        if (root.contains("ui_settings")) {
            QJsonObject ui = root["ui_settings"].toObject();
            startFontSize = ui["font_size"].toInt();

            QString colorStr = ui["background_color"].toString();
            QColor parsedColor(colorStr);
            if (parsedColor.isValid()) {
                startBgColor = parsedColor;
            }
            else {
                QStringList rgb = colorStr.split(",");
                if (rgb.size() == 3) {
                    startBgColor = QColor(rgb[0].trimmed().toInt(), rgb[1].trimmed().toInt(), rgb[2].trimmed().toInt());
                }
            }
        }
        file.close();
    }

    // Применяем настройки, которые только что прочитали из файла
    unifiedView->ApplyUiSettings(loadedFontFamily, startFontSize, startBgColor);

    // 4. Компонуем окно
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->addWidget(unifiedView);

    setCentralWidget(centralWidget);
    centralWidget->adjustSize();
    resize(centralWidget->sizeHint());

    // 5. Панель инструментов
    QToolBar* toolbar = addToolBar("Tools");
    QPushButton* sortBtn = new QPushButton("Автосортировка", this);
    toolbar->addWidget(sortBtn);
    connect(sortBtn, &QPushButton::clicked, this, &MainWindow::onAutoSort);

    QPushButton* settingsBtn = new QPushButton("Настройки", this);
    toolbar->addWidget(settingsBtn);

    connect(settingsBtn, &QPushButton::clicked, this, [this]() {
        SettingsDialog dlg(this);

        connect(&dlg, &SettingsDialog::settingsApplied, this, [this]() {
            QString configPath = QCoreApplication::applicationDirPath() + "/config.json";
            QFile file(configPath);
            if (file.open(QIODevice::ReadOnly)) {
                QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
                QJsonObject root = doc.object();

                if (root.contains("ui_settings")) {
                    QJsonObject ui = root["ui_settings"].toObject();
                    QString bgColorStr = ui["bg_color"].toString();
                    int fontSize = ui["font_size"].toInt();

                    // --- ИСПРАВЛЕНО: Безопасный парсинг цвета внутри лямбды ---
                    QColor newBgColor(bgColorStr);
                    if (!newBgColor.isValid()) {
                        QStringList rgb = bgColorStr.split(",");
                        if (rgb.size() == 3) {
                            newBgColor = QColor(rgb[0].trimmed().toInt(), rgb[1].trimmed().toInt(), rgb[2].trimmed().toInt());
                        }
                        else {
                            newBgColor = QColor(240, 240, 240); // если совсем всё сломалось
                        }
                    }

                    unifiedView->ApplyUiSettings(loadedFontFamily, fontSize, newBgColor);
                }
                file.close();
            }
            unifiedView->UpdateView();
            });

        dlg.exec();
        });
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

    auto parseContainer = [&](const QJsonArray& itemsArray, BackpackLogick* targetLogic) {
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
            else {
                bool placedInStorage = false;
                for (int y = 0; y < 8; ++y) {
                    for (int x = 0; x < 3; ++x) {
                        if (storageLog->CanPlaceItem(item, x, y)) {
                            storageLog->AddItem(item, x, y);
                            placedInStorage = true;
                            break;
                        }
                    }
                    if (placedInStorage) break;
                }

                if (!placedInStorage) {
                    qDebug() << "Предмет" << QString::fromStdString(item.NameItem) << "не влез даже в хранилище!";
                }
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
#include "BackpackView.hpp"
#include <QKeyEvent>
#include <QPainter>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <cmath>
#include <QCoreApplication>
#include <QTimer>
#include <QFont>

class InventoryItemGraphics : public QGraphicsRectItem {
private:
    Item itemData;
    int cellSize;
    BackpackLogick* sourceLogic;
    BackpackView* parentView;
    QString currentFontFamily = "Segoe UI";
    int currentFontSize = 9;
    QColor currentBgColor = QColor(240, 240, 240);

public:
    InventoryItemGraphics(const Item& item, int size, BackpackLogick* sl, BackpackView* view, int regX, int regY)
        : itemData(item), cellSize(size), sourceLogic(sl), parentView(view)
    {
        setRect(0, 0, item.wth * cellSize, item.len * cellSize);
        setPos(regX + item.startX * cellSize, regY + item.startY * cellSize);

        QString exeDir = QCoreApplication::applicationDirPath();
        QString fullTexturePath = exeDir + "/" + QString::fromStdString(item.texturePath);
        QPixmap pixmap(fullTexturePath);

        if (!pixmap.isNull()) {
            QTransform transform;
            transform.rotate(item.rotation * 90);
            QPixmap rotatedPixmap = pixmap.transformed(transform, Qt::SmoothTransformation);

            QPixmap scaledPixmap = rotatedPixmap.scaled(item.wth * cellSize, item.len * cellSize,
                Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

            setBrush(QBrush(scaledPixmap));
            setPen(QPen(Qt::transparent));
        }
        else {
            QColor color = item.rotatable ? QColor(100, 150, 255, 200) : QColor(255, 100, 100, 200);
            setBrush(QBrush(color));
            setPen(QPen(Qt::black, 1));
        }

        setToolTip(QString::fromStdString(item.NameItem));
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemIsFocusable);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        setAcceptHoverEvents(false);
        setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);

        setZValue(1);
    }

    int GetItemID() const { return itemData.ID; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        if (event->button() == Qt::RightButton) {
            sourceLogic->RemoveItem(itemData.startX, itemData.startY);
            parentView->UpdateView();
            return;
        }
        setFocus();
        QGraphicsRectItem::mousePressEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
        QGraphicsRectItem::mouseReleaseEvent(event);

        qreal sceneX = this->scenePos().x();
        qreal sceneY = this->scenePos().y();

        BackpackLogick* targetLogic = nullptr;
        int localGridX = -1;
        int localGridY = -1;

        for (const auto& reg : parentView->GetRegions()) {
            qreal rMinX = reg.offsetX;
            qreal rMaxX = reg.offsetX + reg.widthCells * cellSize;
            qreal rMinY = reg.offsetY;
            qreal rMaxY = reg.offsetY + reg.heightCells * cellSize;

            if (sceneX >= rMinX - cellSize / 2.0 && sceneX < rMaxX &&
                sceneY >= rMinY - cellSize / 2.0 && sceneY < rMaxY) {

                localGridX = std::round((sceneX - reg.offsetX) / cellSize);
                localGridY = std::round((sceneY - reg.offsetY) / cellSize);
                targetLogic = reg.logic;
                break;
            }
        }

        if (targetLogic) {
            if (targetLogic == sourceLogic) {
                sourceLogic->MoveItem(itemData.startX, itemData.startY, localGridX, localGridY);
            }
            else {
                if (targetLogic->CanPlaceItem(itemData, localGridX, localGridY)) {
                    sourceLogic->RemoveItem(itemData.startX, itemData.startY);
                    targetLogic->AddItem(itemData, localGridX, localGridY);
                }
            }
        }

        QTimer::singleShot(0, parentView, &BackpackView::UpdateView);
    }

    void keyPressEvent(QKeyEvent* event) override {
        QString pressedChar = event->text().toLower();

        if (pressedChar == "r" || pressedChar == "к") {
            if (sourceLogic->RotateItem(itemData.startX, itemData.startY)) {
                QTimer::singleShot(0, parentView, &BackpackView::UpdateView);
            }
        }
        else {
            QGraphicsRectItem::keyPressEvent(event);
        }
    }
};


BackpackView::BackpackView(BackpackLogick* mainLog, BackpackLogick* beltLog, BackpackLogick* handsLog, BackpackLogick* storageLog, QWidget* parent)
    : QGraphicsView(parent), cellSize(50)
{
    scene = new QGraphicsScene(this);
    setScene(scene);

    // Новая монолитная карта расположения с учетом Хранилища слева
    // Параметры: { Логика, Название, ОффсетX, ОффсетY, Кол-во ячеек X, Кол-во ячеек Y, ФлагГлавногоРюкзака }
    regions.append({ storageLog, "ХРАНИЛИЩЕ",         30,  50,  3, 8, false }); // Высокое хранилище (3х8)
    regions.append({ handsLog,   "В РУКАХ",          220, 150,  2, 2, false }); // По центру чуть ниже (2х2)
    regions.append({ mainLog,    "ИНВЕНТАРЬ",        360,  50,  5, 6, true });  // Основной рюкзак (5х6)
    regions.append({ beltLog,    "ПОЯСНЫЕ ПОДСУМКИ", 385, 400,  4, 1, false }); // Под основным рюкзаком (4х1)

    // Расширили экран под новые размеры (Ширина: 660, Высота: 500)
    scene->setSceneRect(0, 0, 660, 500);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMinimumSize(662, 502);
    setBackgroundBrush(QColor(27, 4, 236));
    UpdateView();
}

void BackpackView::drawBackground(QPainter* painter, const QRectF& rect)
{
    painter->save();

    QGraphicsView::drawBackground(painter, rect);
    painter->setRenderHint(QPainter::Antialiasing);

    for (const auto& reg : regions)
    {
        painter->setBrush(Qt::NoBrush);

        QColor textColor = (currentBgColor.lightness() < 128) ? Qt::white : Qt::black;
        // --- ОТРИСОВКА НАДПИСЕЙ ---
        painter->setPen(QPen(textColor, 1));
        painter->setFont(QFont(currentFontFamily, currentFontSize, QFont::Bold));
        painter->drawText(reg.offsetX, reg.offsetY - 10, reg.title);

        // --- ОТРИСОВКА СЕТКИ ---
        painter->setPen(QPen(QColor(100, 100, 100, 50), 1));
        for (int i = 0; i <= reg.widthCells; ++i) {
            painter->drawLine(reg.offsetX + i * cellSize, reg.offsetY,
                reg.offsetX + i * cellSize, reg.offsetY + reg.heightCells * cellSize);
        }
        for (int j = 0; j <= reg.heightCells; ++j) {
            painter->drawLine(reg.offsetX, reg.offsetY + j * cellSize,
                reg.offsetX + reg.widthCells * cellSize, reg.offsetY + j * cellSize);
        }

        // --- ОТРИСОВКА РАМОК ---
        QColor frameColor = Qt::gray;
        if (reg.isMainBackpack) {
            frameColor = reg.logic->IsBalance() ? Qt::green : Qt::red;
        }

        painter->setPen(QPen(frameColor, 3));
        painter->drawRect(reg.offsetX, reg.offsetY,
            reg.widthCells * cellSize, reg.heightCells * cellSize);
    }

    painter->restore();
}

void BackpackView::UpdateView()
{
    int focusedID = -1;
    if (scene->focusItem()) {
        InventoryItemGraphics* currentFocus = static_cast<InventoryItemGraphics*>(scene->focusItem());
        if (currentFocus) {
            focusedID = currentFocus->GetItemID();
        }
    }

    scene->clear();

    for (const auto& reg : regions) {
        for (const auto& item : reg.logic->GetItems()) {
            if (item.startX < 0 || item.startY < 0) continue;

            InventoryItemGraphics* itemGraphics = new InventoryItemGraphics(item, cellSize, reg.logic, this, reg.offsetX, reg.offsetY);
            scene->addItem(itemGraphics);

            if (item.ID == focusedID) {
                itemGraphics->setFocus();
            }
        }
    }

    scene->invalidate(scene->sceneRect(), QGraphicsScene::BackgroundLayer);
}

void BackpackView::ApplyUiSettings(const QString& fontFamily, int fontSize, const QColor& bgColor)
{
    currentFontFamily = fontFamily;
    currentFontSize = fontSize;
    currentBgColor = bgColor;

    // Сразу обновляем фон самого представления
    setBackgroundBrush(currentBgColor);
}
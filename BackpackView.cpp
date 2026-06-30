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

class InventoryItemGraphics : public QGraphicsRectItem {
private:
    Item itemData;
    int cellSize;
    BackpackLogick* sourceLogic; // Контейнер, откуда взят предмет
    BackpackView* parentView;

public:
    InventoryItemGraphics(const Item& item, int size, BackpackLogick* sl, BackpackView* view, int regX, int regY)
        : itemData(item), cellSize(size), sourceLogic(sl), parentView(view)
    {
        setRect(0, 0, item.wth * cellSize, item.len * cellSize);
        // Позиция на сцене = смещение контейнера + локальные координаты предмета
        setPos(regX + item.startX * cellSize, regY + item.startY * cellSize);

        QString exeDir = QCoreApplication::applicationDirPath();
        QString fullTexturePath = exeDir + "/" + QString::fromStdString(item.texturePath);
        QPixmap pixmap(fullTexturePath);

        if (!pixmap.isNull()) {
            // 1. Сначала поворачиваем ИСХОДНУЮ картинку на нужный угол
            QTransform transform;
            transform.rotate(item.rotation * 90);
            QPixmap rotatedPixmap = pixmap.transformed(transform, Qt::SmoothTransformation);

            // 2. Затем масштабируем УЖЕ ПОВЕРНУТУЮ картинку под текущие width и length предмета
            QPixmap scaledPixmap = rotatedPixmap.scaled(item.wth * cellSize, item.len * cellSize,
                Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

            setBrush(QBrush(scaledPixmap));
            setPen(QPen(Qt::transparent));
        }
        else {
            // Для заглушек без текстур можно визуально ничего не крутить, просто менять размер
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

        setZValue(1); // Предметы будут всегда поверх фона, но под всплывающими подсказками
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

        // БЕЗОПАСНЫЙ ВЫЗОВ ОБНОВЛЕНИЯ
        QTimer::singleShot(0, parentView, &BackpackView::UpdateView);
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_R) {
            if (sourceLogic->RotateItem(itemData.startX, itemData.startY)) {
                // Здесь тоже безопасный вызов
                QTimer::singleShot(0, parentView, &BackpackView::UpdateView);
            }
        }
        else {
            QGraphicsRectItem::keyPressEvent(event);
        }
    }
};

BackpackView::BackpackView(BackpackLogick* mainLog, BackpackLogick* beltLog, BackpackLogick* handsLog, QWidget* parent)
    : QGraphicsView(parent), cellSize(50)
{
    scene = new QGraphicsScene(this);
    setScene(scene);

    // Задаем фиксированную карту смещений для компактного монолитного интерфейса
    regions.append({ handsLog, "В РУКАХ", 40, 130, 2, 2, false });          // Слева
    regions.append({ mainLog, "ИНВЕНТАРЬ", 180, 40, 5, 6, true });          // По центру (5х6)
    regions.append({ beltLog, "ПОЯСНЫЕ ПОДСУМКИ", 205, 380, 4, 1, false }); // Снизу горизонтально (4х1)

    scene->setSceneRect(0, 0, 480, 460);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMinimumSize(482, 462);
    setBackgroundBrush(QColor(240, 240, 240)); // Легкий игровой серый фон
    UpdateView();
}

void BackpackView::drawBackground(QPainter* painter, const QRectF& rect)
{
    // 1. Сохраняем состояние рисовальщика перед началом
    painter->save();

    // Рисуем базовый фон QGraphicsView
    QGraphicsView::drawBackground(painter, rect);
    painter->setRenderHint(QPainter::Antialiasing);

    for (const auto& reg : regions)
    {
        // 2. Сбрасываем Pen/Brush для каждого блока, чтобы настройки не "текли"
        painter->setBrush(Qt::NoBrush);

        // Рисуем сетку
        painter->setPen(QPen(QColor(100, 100, 100, 50), 1));
        for (int i = 0; i <= reg.widthCells; ++i) {
            painter->drawLine(reg.offsetX + i * cellSize, reg.offsetY,
                reg.offsetX + i * cellSize, reg.offsetY + reg.heightCells * cellSize);
        }
        for (int j = 0; j <= reg.heightCells; ++j) {
            painter->drawLine(reg.offsetX, reg.offsetY + j * cellSize,
                reg.offsetX + reg.widthCells * cellSize, reg.offsetY + j * cellSize);
        }

        // 3. Рисуем рамку
        QColor frameColor = Qt::gray;
        if (reg.isMainBackpack) {
            // Если баланс нарушен — красный, если ок — зеленый
            frameColor = reg.logic->IsBalance() ? Qt::green : Qt::red;
        }

        painter->setPen(QPen(frameColor, 3));
        painter->drawRect(reg.offsetX, reg.offsetY,
            reg.widthCells * cellSize, reg.heightCells * cellSize);
    }

    // 4. Возвращаем рисовальщик в исходное состояние
    painter->restore();
}

void BackpackView::UpdateView()
{
    // 1. Запоминаем ID предмета, который сейчас выбран (в фокусе)
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

            // 2. Возвращаем фокус предмету, чтобы можно было жать 'R' подряд!
            if (item.ID == focusedID) {
                itemGraphics->setFocus();
            }
        }
    }

    // Принудительно чистим кэш фона (ты это уже сделал, оставляем)
    scene->invalidate(scene->sceneRect(), QGraphicsScene::BackgroundLayer);
}
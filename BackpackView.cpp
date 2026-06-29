#include "BackpackView.hpp"
#include <QKeyEvent>
#include <QPainter>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <cmath>

class InventoryItemGraphics : public QGraphicsRectItem {
private:
    Item itemData;                 // Данные о предмете
    int cellSize;                  // Размер ячейки в пикселях
    BackpackLogick* logic;         // Ссылка на бэкенд логики
    BackpackView* parentView;      // Ссылка на представление для обновления экрана

public:
    InventoryItemGraphics(const Item& item, int size, BackpackLogick* bl, BackpackView* view)
        : itemData(item), cellSize(size), logic(bl), parentView(view)
    {
        // Отрисовка геометрии прямоугольника (от нуля до размеров предмета)
        setRect(0, 0, item.wth * cellSize, item.len * cellSize);

        // Установка позиции на сцене в пикселях
        setPos(item.startX * cellSize, item.startY * cellSize);

        // Цветовое оформление
        QColor color = item.rotatable ? QColor(100, 150, 255, 200) : QColor(255, 100, 100, 200);
        setBrush(QBrush(color));
        setPen(QPen(Qt::black, 1));
        setToolTip(QString::fromStdString(item.NameItem));

        // ВКЛЮЧАЕМ встроенную подвижность Qt
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemIsFocusable);
    }

protected:
    // ЕДИНСТВЕННЫЙ mousePressEvent (объединили фокус и удаление)
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        // УДАЛЕНИЕ ПРЕДМЕТА НА ПРАВЫЙ КЛИК
        if (event->button() == Qt::RightButton) {
            logic->RemoveItem(itemData.startX, itemData.startY);
            parentView->UpdateView();
            return;
        }

        // Для левого клика (перетаскивания) просто забираем фокус для клавиши R
        setFocus();
        QGraphicsRectItem::mousePressEvent(event);
    }

    // Переопределяем событие отпускания мыши (Момент сброса предмета)
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
        QGraphicsRectItem::mouseReleaseEvent(event);

        qreal pixelX = this->scenePos().x();
        qreal pixelY = this->scenePos().y();

        int newGridX = std::round(pixelX / cellSize);
        int newGridY = std::round(pixelY / cellSize);

        bool success = logic->MoveItem(itemData.startX, itemData.startY, newGridX, newGridY);

        if (success) {
            itemData.startX = newGridX;
            itemData.startY = newGridY;
        }

        parentView->UpdateView();
    }

    // Обработка клавиатуры (классический поворот лежащего предмета)
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_R) {
            bool success = logic->RotateItem(itemData.startX, itemData.startY);

            if (success) {
                parentView->UpdateView();
            }
        }
        else {
            QGraphicsRectItem::keyPressEvent(event);
        }
    }
};


BackpackView::BackpackView(BackpackLogick* logic, QWidget* parent)
    : QGraphicsView(parent), backpack(logic), cellSize(50)
{
    scene = new QGraphicsScene(this);
    setScene(scene);
    scene->setSceneRect(0, 0, backpack->GetWidth() * cellSize, backpack->GetLength() * cellSize);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMinimumSize(backpack->GetWidth() * cellSize + 2, backpack->GetLength() * cellSize + 2);
    setBackgroundBrush(Qt::white);
    UpdateView();
}


void BackpackView::drawBackground(QPainter* painter, const QRectF& rect)
{
    QGraphicsView::drawBackground(painter, rect);

    // 1. Отрисовка внутренней сетки (как у тебя и было)
    painter->setPen(QPen(Qt::gray, 1));
    for (int row = 0; row <= backpack->GetLength(); ++row)
        painter->drawLine(0, row * cellSize, backpack->GetWidth() * cellSize, row * cellSize);
    for (int col = 0; col <= backpack->GetWidth(); ++col)
        painter->drawLine(col * cellSize, 0, col * cellSize, backpack->GetLength() * cellSize);

    // 2. ВИЗУАЛИЗАЦИЯ БАЛАНСА
    // Проверяем твой метод IsBalance()
    if (backpack->IsBalance()) {
        // Баланс в норме (или рюкзак пуст) — рисуем нейтральную или зеленую рамку
        painter->setPen(QPen(Qt::green, 3));
    }
    else {
        // Перевес — привлекаем внимание красной рамкой
        painter->setPen(QPen(Qt::red, 3));
    }

    // Отрисовываем контур вокруг всего рюкзака
    painter->drawRect(0, 0, backpack->GetWidth() * cellSize, backpack->GetLength() * cellSize);
}


void BackpackView::UpdateView()
{
    scene->clear(); // Очищаем старые прямоугольники перед перерисовкой

    for (const auto& item : backpack->GetItems()) {
        if (item.startX < 0 || item.startY < 0) continue;

        // Создаем экземпляр нашего кастомного класса
        InventoryItemGraphics* itemGraphics = new InventoryItemGraphics(item, cellSize, backpack, this);

        // Добавляем его на сцену
        scene->addItem(itemGraphics);
    }
}
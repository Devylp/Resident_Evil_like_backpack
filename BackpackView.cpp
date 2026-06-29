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
    bool isHeld = false;

    // Переопределяем событие отпускания мыши (Момент сброса предмета)
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
        // Передаем событие базовому классу, чтобы Qt обновил внутренние координаты
        QGraphicsRectItem::mouseReleaseEvent(event);

        // Получаем пиксельные координаты левого верхнего угла предмета на сцене
        qreal pixelX = this->scenePos().x();
        qreal pixelY = this->scenePos().y();

        // Переводим пиксели обратно в индексы ячеек матрицы рюкзака
        int newGridX = std::round(pixelX / cellSize);
        int newGridY = std::round(pixelY / cellSize);

        // Обращаемся к твоей родной бэкенд-логике перемещения
        bool success = logic->MoveItem(itemData.startX, itemData.startY, newGridX, newGridY);

        if (success) {
            // Если MoveItem вернул true (ячейки свободны, перевеса нет), сохраняем новые координаты
            itemData.startX = newGridX;
            itemData.startY = newGridY;
        }

        // Вызываем полную перерисовку. Если перемещение было запрещено логикой,
        // UpdateView просто нарисует предмет на его старых валидных координатах (он отскочит назад).
        parentView->UpdateView();
    }

    // 1. При клике на предмет принудительно забираем фокус клавиатуры на него
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        setFocus(); // Теперь этот предмет слушает клавиатуру
        QGraphicsRectItem::mousePressEvent(event);
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_R) {
            if (isHeld) {
                // Предмет в руке 
                std::swap(itemData.wth, itemData.len);
                this->setRect(0, 0, itemData.wth * cellSize, itemData.len * cellSize);
            }
            else {
                // Предмет лежит в рюкзаке.
                bool success = logic->RotateItem(itemData.startX, itemData.startY);
                if (success) {
                    parentView->UpdateView();
                }
            }
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
    painter->setPen(QPen(Qt::gray, 1));
    for (int row = 0; row <= backpack->GetLength(); ++row)
        painter->drawLine(0, row * cellSize, backpack->GetWidth() * cellSize, row * cellSize);
    for (int col = 0; col <= backpack->GetWidth(); ++col)
        painter->drawLine(col * cellSize, 0, col * cellSize, backpack->GetLength() * cellSize);
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
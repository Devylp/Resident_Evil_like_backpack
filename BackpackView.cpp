#include "BackpackView.hpp"
#include <QPainter>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>

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
    scene->clear();
    for (const auto& item : backpack->GetItems()) {
        if (item.startX < 0 || item.startY < 0) continue;
        QColor color = item.rotatable ? QColor(100, 150, 255, 200) : QColor(255, 100, 100, 200);
        QGraphicsRectItem* rectItem = scene->addRect(
            item.startX * cellSize, item.startY * cellSize,
            item.wth * cellSize, item.len * cellSize,
            QPen(Qt::black, 1), QBrush(color)
        );
        rectItem->setData(0, item.ID);
        rectItem->setToolTip(QString::fromStdString(item.NameItem));
    }
}
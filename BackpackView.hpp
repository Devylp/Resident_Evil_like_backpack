#pragma once
#include <QGraphicsView>
#include "BackpackLogick.hpp"

class BackpackView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit BackpackView(BackpackLogick* logic, QWidget* parent = nullptr);

    void UpdateView();

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    BackpackLogick* backpack;
    QGraphicsScene* scene;
    int cellSize;
};
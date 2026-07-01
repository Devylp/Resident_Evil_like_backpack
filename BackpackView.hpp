    #pragma once
    #include <QGraphicsView>
    #include <QGraphicsScene>
    #include <QVector>
    #include <QString>
    #include "BackpackLogick.hpp"

    // Структура для описания региона контейнера на единой сцене
    struct ContainerRegion {
        BackpackLogick* logic;
        QString title;
        int offsetX;
        int offsetY;
        int widthCells;
        int heightCells;
        bool isMainBackpack;
    };

    class BackpackView : public QGraphicsView
    {
        Q_OBJECT

    public:
        explicit BackpackView(BackpackLogick* mainLog, BackpackLogick* beltLog, BackpackLogick* handsLog, BackpackLogick* storageLog, QWidget* parent = nullptr);
        void UpdateView();
        void ApplyUiSettings(const QString& fontFamily, int fontSize, const QColor& bgColor);
        const QVector<ContainerRegion>& GetRegions() const { return regions; }
        int GetCellSize() const { return cellSize; }

    protected:
        void drawBackground(QPainter* painter, const QRectF& rect) override;

    private:
        QGraphicsScene* scene;
        int cellSize;
        QVector<ContainerRegion> regions;
        QString currentFontFamily = "Segoe UI";
        int currentFontSize = 9;
        QColor currentBgColor = QColor(240, 240, 240);
    };
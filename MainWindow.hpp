#pragma once
#include <QMainWindow>
#include <QString>

class BackpackLogick;
class BackpackView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onAutoSort();

private:
    void loadFromJson(const QString& filePath);

    QString loadedFontFamily = "Segoe UI";

    BackpackLogick* mainLogic;
    BackpackLogick* beltLogic;
    BackpackLogick* handsLogic;
    BackpackLogick* storageLog;

    BackpackView* unifiedView; // Одно представление вместо трёх!
};
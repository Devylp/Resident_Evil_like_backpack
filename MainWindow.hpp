#pragma once
#include <QMainWindow>
#include <QString>

// Предварительное объявление классов
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
    // Метод для загрузки данных из JSON
    void loadFromJson(const QString& filePath);

    // --- ЛОГИКА ---
    BackpackLogick* mainLogic;  // Главный рюкзак
    BackpackLogick* beltLogic;  // Пояс
    BackpackLogick* handsLogic; // Руки

    // --- ОТОБРАЖЕНИЕ ---
    // Пока сделаем один View для главного рюкзака, чтобы не усложнять окно прямо сейчас
    BackpackView* mainView;
};
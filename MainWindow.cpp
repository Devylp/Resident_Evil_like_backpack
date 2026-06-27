#include "MainWindow.hpp"
#include "BackpackLogick.hpp"   // Логика рюкзака (пока не используется, но скоро)
#include "BackpackView.hpp"     // Графическая сцена рюкзака (пока закомментируем)

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Пока оставляем окно пустым.
    // Завтра вставим сюда создание BackpackLogick и BackpackView.
    setWindowTitle("Рюкзак");
    resize(600, 500);           // Стартовый размер окна
}
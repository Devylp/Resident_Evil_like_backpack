#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);             // 1.   Объект приложения
    MainWindow w;                   // 2. Создаём главное окно
    w.show();                       // 3. Показываем его

    return app.exec();              // 4. Запускаем цикл обработки событий
}
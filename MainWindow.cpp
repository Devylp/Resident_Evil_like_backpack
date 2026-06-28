#include "MainWindow.hpp"
#include "BackpackLogick.hpp"
#include "BackpackView.hpp"
#include "Item.hpp"
#include <QToolBar>
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Backpack");

    auto* logic = new BackpackLogick(6, 5, 100);

    Item sword;
    sword.ID = 1;
    sword.NameItem = "Sword";
    sword.weight = 5;
    sword.len = 2;
    sword.wth = 1;
    sword.rotatable = true;
    logic->AddItem(sword, 0, 0);

    Item shield;
    shield.ID = 2;
    shield.NameItem = "Shield";
    shield.weight = 8;
    shield.len = 2;
    shield.wth = 2;
    shield.rotatable = false;
    logic->AddItem(shield, 2, 0);

    auto* view = new BackpackView(logic, this);
    setCentralWidget(view);

    resize(view->sizeHint());

    // ---------- Вот здесь размещается кнопка ----------
    QToolBar* toolbar = addToolBar("Tools");
    QPushButton* sortBtn = new QPushButton("Auto Sort", this);
    toolbar->addWidget(sortBtn);

    // connect пока закомментируем, так как слот ещё не объявлен
    // connect(sortBtn, &QPushButton::clicked, this, &MainWindow::onAutoSort);
}

void MainWindow::onAutoSort()
{
    auto notPlaced = logic->AutoSortBackpack();   // вызываем логику
    view->UpdateView();                           // обновляем экран
    // Позже обработаем notPlaced (диалог с непринятыми предметами)
}
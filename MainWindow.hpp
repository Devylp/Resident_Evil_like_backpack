#pragma once
#include <QMainWindow>

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
    BackpackLogick* logic;   // указатель на логику (живёт всё время)
    BackpackView* view;    // указатель на графический вид (живёт всё время)
};
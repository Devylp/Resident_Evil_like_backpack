#pragma once
#include <QDialog>
#include <QJsonObject>

class QVBoxLayout;
class QPushButton;
class QSpinBox;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    QJsonObject GetCurrentSettings() const;

signals:
    void settingsApplied(); // Сигнал для MainWindow обновить интерфейс

private slots:
    void chooseBgColor();
    void saveToJson();

private:
    QColor currentBgColor;
    QPushButton* colorBtn;
    QSpinBox* fontSizeSpin;
};
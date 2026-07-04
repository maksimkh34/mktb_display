#pragma once
#include "ScaleView.h"
#include <QMainWindow>
#include <QLabel>
#include <QHBoxLayout>
#include <QPixmap>

class MainWindow : public QMainWindow, public ScaleView {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    // Реализация интерфейса ScaleView
    void updateWeight(double weight) override;
    void updateUnit(const QString &unit) override;
    void updateTareState(bool isActive) override;
    void displayError(const QString &error) override;

private:
    QLabel *m_tareIconLabel;
    QLabel *m_weightDisplayLabel;
    QLabel *m_unitLabel;

    QPixmap m_tareOnPixmap;
    QPixmap m_tareOffPixmap;
};
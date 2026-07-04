 #include "MainWindow.h"

#include <QWidget>

#include <QHBoxLayout>

#include <QVBoxLayout>

#include <QStatusBar>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

// Установка минимального размера окна

setFixedSize(360, 112);


auto *centralWidget = new QWidget(this);

auto *mainVLayout = new QVBoxLayout(centralWidget);


// Центрируем всё содержимое главного контейнера по горизонтали и прижимаем к верху

mainVLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);


// Внешние поля главного окна (отступы от краев рамки до контента)

mainVLayout->setContentsMargins(15, 5, 15, 10);

mainVLayout->setSpacing(0);


// =========================================================================

// КОНТЕЙНЕР 1: ВЕРХНЯЯ НАДПИСЬ

// =========================================================================

auto *titleLabel = new QLabel("Весы №2 468", this);

QFont titleFont = titleLabel->font();

titleFont.setPointSize(11);

titleFont.setBold(true);

titleLabel->setFont(titleFont);

titleLabel->setAlignment(Qt::AlignCenter);


auto *titleContainer = new QVBoxLayout();

// РЕГУЛИРОВКА ОТСТУПОВ НАДПИСИ: (Слева, Сверху, Справа, Снизу)

titleContainer->setContentsMargins(0, 0, 0, 12);

titleContainer->addWidget(titleLabel);


// Добавляем первый контейнер в главный вертикальный слой

mainVLayout->addLayout(titleContainer);



// =========================================================================

// КОНТЕЙНЕР 2: ОСНОВНЫЕ ЭЛЕМЕНТЫ (ТАРА, ВЕС, ЕД. ИЗМ.)

// =========================================================================

auto *rowLayout = new QHBoxLayout();

rowLayout->setSpacing(0);

// Выравнивание самого горизонтального ряда по центру

rowLayout->setAlignment(Qt::AlignCenter);


// 1. Индикатор тары

m_tareIconLabel = new QLabel(this);

const int tareSize = 34;

m_tareIconLabel->setFixedSize(tareSize, tareSize);

m_tareIconLabel->setScaledContents(true);


m_tareOnPixmap = QPixmap(tareSize, tareSize);

m_tareOnPixmap.fill(Qt::green);

m_tareOffPixmap = QPixmap(tareSize, tareSize);

m_tareOffPixmap.fill(Qt::gray);

m_tareIconLabel->setPixmap(m_tareOffPixmap);


auto *tareContainer = new QVBoxLayout();

// РЕГУЛИРОВКА ОТСТУПОВ ТАРЫ: (Слева, Сверху, Справа, Снизу)

tareContainer->setContentsMargins(5, 0, 10, 0);

// Qt::AlignVCenter принудительно центрирует элемент по вертикали внутри его контейнера

tareContainer->addWidget(m_tareIconLabel, 0, Qt::AlignVCenter | Qt::AlignHCenter);

rowLayout->addLayout(tareContainer);


// 2. Дисплей веса

m_weightDisplayLabel = new QLabel("0.000", this);

QFont weightFont = m_weightDisplayLabel->font();

weightFont.setPointSize(34);

weightFont.setBold(true);

m_weightDisplayLabel->setFont(weightFont);

m_weightDisplayLabel->setAlignment(Qt::AlignCenter);


auto *weightContainer = new QVBoxLayout();

// РЕГУЛИРОВКА ОТСТУПОВ ВЕСА: (Слева, Сверху, Справа, Снизу)

weightContainer->setContentsMargins(10, 0, 10, 6);

weightContainer->addWidget(m_weightDisplayLabel, 0, Qt::AlignVCenter | Qt::AlignHCenter);

rowLayout->addLayout(weightContainer);


// 3. Единицы измерения

m_unitLabel = new QLabel("кг", this);

QFont unitFont = m_unitLabel->font();

unitFont.setPointSize(18);

m_unitLabel->setFont(unitFont);

m_unitLabel->setAlignment(Qt::AlignCenter);


auto *unitContainer = new QVBoxLayout();

// РЕГУЛИРОВКА ОТСТУПОВ ЕДИНЦ ИЗМЕРЕНИЯ: (Слева, Сверху, Справа, Снизу)

// Изменили верхний отступ с 15 на 0, теперь элемент центрируется строго по вертикали

unitContainer->setContentsMargins(10, 0, 5, 0);

unitContainer->addWidget(m_unitLabel, 0, Qt::AlignVCenter | Qt::AlignHCenter);

rowLayout->addLayout(unitContainer);


// Добавляем второй собранный контейнер (ряд) в главный вертикальный слой

mainVLayout->addLayout(rowLayout);


setCentralWidget(centralWidget);

}


void MainWindow::updateWeight(double weight) {

m_weightDisplayLabel->setText(QString::number(weight, 'f', 3));

}


void MainWindow::updateUnit(const QString &unit) {

m_unitLabel->setText(unit);

}


void MainWindow::updateTareState(bool isActive) {

m_tareIconLabel->setPixmap(isActive ? m_tareOnPixmap : m_tareOffPixmap);

}


void MainWindow::displayError(const QString &error) {

statusBar()->showMessage(error, 5000);

} 
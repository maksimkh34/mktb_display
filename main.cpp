#include <QApplication>
#include "MainWindow.h"
#include "MassaKProtocol2.h"
#include "ScalePresenter.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 1. Создаем UI слой
    MainWindow view;
    view.show();

    // 2. Создаем слой железа (сейчас это Massa-K по Протоколу №2)
    ScaleDevice *device = new MassaKProtocol2(&a);

    // 3. Создаем Presenter, связывающий их
    ScalePresenter presenter(&view, device, &a);

    // 4. Инициализируем подключение (замените "COM3" / "/dev/ttyUSB0" на ваш порт)
    device->connectDevice("COM3");

    return QApplication::exec();
}
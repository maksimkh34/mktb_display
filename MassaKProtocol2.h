#pragma once
#include "ScaleDevice.h"
#include <QSerialPort>
#include <QTimer>

class MassaKProtocol2 : public ScaleDevice {
    Q_OBJECT
public:
    explicit MassaKProtocol2(QObject *parent = nullptr);
    ~MassaKProtocol2() override;

    void connectDevice(const QString &portName) override;
    void disconnectDevice() override;
    void requestData() override;

private slots:
    void handleReadyRead();
    void sendNextCommand();

private:
    QSerialPort *m_serial;
    QTimer *m_pollTimer;
    enum class CommandState { Idle, WaitingStatus, WaitingWeight };
    CommandState m_state;
    ScaleData m_currentData;
};
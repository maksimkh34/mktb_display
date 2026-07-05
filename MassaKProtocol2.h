#pragma once
#include "ScaleDevice.h"
#include <QSerialPort>
#include <QByteArray>
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
    void handleResponseTimeout();
    void tryAutoConnect();

private:
    enum class CommandState { Idle, WaitingStatus, WaitingWeight };

    bool openPort(const QString &portName);
    void closePort();
    void sendCommand(char command, CommandState nextState);

    QSerialPort *m_serial;
    QTimer *m_pollTimer;
    QTimer *m_responseTimer;
    QTimer *m_reconnectTimer;
    CommandState m_state;
    ScaleData m_currentData;
    QByteArray m_rxBuffer;
    bool m_autoConnect;
    int m_consecutiveTimeouts;
};

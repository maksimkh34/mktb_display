#include "MassaKProtocol2.h"

MassaKProtocol2::MassaKProtocol2(QObject *parent)
    : ScaleDevice(parent), m_serial(new QSerialPort(this)), m_pollTimer(new QTimer(this)), m_state(CommandState::Idle) {
    
    connect(m_serial, &QSerialPort::readyRead, this, &MassaKProtocol2::handleReadyRead);
    connect(m_pollTimer, &QTimer::timeout, this, &MassaKProtocol2::sendNextCommand);
}

MassaKProtocol2::~MassaKProtocol2() {
    MassaKProtocol2::disconnectDevice();
}

void MassaKProtocol2::connectDevice(const QString &portName) {
    m_serial->setPortName(portName);
    m_serial->setBaudRate(QSerialPort::Baud4800); // По спецификации 4800 бод
    m_serial->setDataBits(QSerialPort::Data8);     // 8 бит данных
    m_serial->setParity(QSerialPort::EvenParity);   // 1 бит контроля по четности
    m_serial->setStopBits(QSerialPort::OneStop);    // 1 стоповый бит

    if (m_serial->open(QIODevice::ReadWrite)) {
        m_pollTimer->start(200); // Опрос раз в 200 мс
    } else {
        emit errorOccurred(m_serial->errorString());
    }
}

void MassaKProtocol2::disconnectDevice() {
    m_pollTimer->stop();
    if (m_serial->isOpen()) {
        m_serial->close();
    }
    m_state = CommandState::Idle;
}

void MassaKProtocol2::sendNextCommand() {
    if (m_state != CommandState::Idle) return;

    m_state = CommandState::WaitingStatus;
    constexpr char cmdStatus = 0x44; // Запрос слова состояния
    m_serial->write(&cmdStatus, 1);
}

void MassaKProtocol2::handleReadyRead() {
    // Ждем минимум 2 байта ответа (согласно описанию, передача всегда состоит из 2-х байт)
    if (m_serial->bytesAvailable() < 2) return;

    const QByteArray response = m_serial->read(2);
    const auto lowByte = static_cast<quint8>(response.at(0));  // Младший байт сначала
    const auto highByte = static_cast<quint8>(response.at(1)); // Старший байт затем

    if (m_state == CommandState::WaitingStatus) {
        // D5 — индикатор «канистра/тара» 
        m_currentData.isTareActive = ((lowByte & 0x20) != 0);

        // Переходим к запросу веса
        m_state = CommandState::WaitingWeight;
        constexpr char cmdWeight = 0x45; // Запрос массы
        m_serial->write(&cmdWeight, 1);
    } 
    else if (m_state == CommandState::WaitingWeight) {
        // High byte D15 (знак массы): 0 — «+», 1 — «-» 
        const bool isNegative = ((highByte & 0x80) != 0);
        // Масса в граммах: биты D14-D0 
        const quint16 rawWeight = ((highByte & 0x7F) << 8) | lowByte;

        double weightInKg = static_cast<double>(rawWeight) / 1000.0;
        if (isNegative) weightInKg = -weightInKg;

        m_currentData.weight = weightInKg;
        m_currentData.unit = "кг";

        emit dataReceived(m_currentData);
        m_state = CommandState::Idle;
    }
}

void MassaKProtocol2::requestData() {
    sendNextCommand();
}
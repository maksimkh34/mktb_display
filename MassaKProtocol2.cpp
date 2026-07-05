#include "MassaKProtocol2.h"

#include <QSerialPortInfo>

#include <algorithm>

MassaKProtocol2::MassaKProtocol2(QObject *parent)
    : ScaleDevice(parent),
      m_serial(new QSerialPort(this)),
      m_pollTimer(new QTimer(this)),
      m_responseTimer(new QTimer(this)),
      m_reconnectTimer(new QTimer(this)),
      m_state(CommandState::Idle),
      m_autoConnect(false),
      m_consecutiveTimeouts(0) {

    m_responseTimer->setSingleShot(true);
    m_responseTimer->setInterval(500);
    m_reconnectTimer->setSingleShot(true);
    m_reconnectTimer->setInterval(1000);

    connect(m_serial, &QSerialPort::readyRead, this, &MassaKProtocol2::handleReadyRead);
    connect(m_pollTimer, &QTimer::timeout, this, &MassaKProtocol2::sendNextCommand);
    connect(m_responseTimer, &QTimer::timeout, this, &MassaKProtocol2::handleResponseTimeout);
    connect(m_reconnectTimer, &QTimer::timeout, this, &MassaKProtocol2::tryAutoConnect);
}

MassaKProtocol2::~MassaKProtocol2() {
    MassaKProtocol2::disconnectDevice();
}

void MassaKProtocol2::connectDevice(const QString &portName) {
    m_autoConnect = portName.trimmed().isEmpty();

    if (m_autoConnect) {
        tryAutoConnect();
        return;
    }

    if (!openPort(portName.trimmed())) {
        emit errorOccurred(QString("Не удалось открыть порт %1: %2")
                               .arg(portName, m_serial->errorString()));
    }
}

void MassaKProtocol2::disconnectDevice() {
    m_autoConnect = false;
    m_reconnectTimer->stop();
    closePort();
}

bool MassaKProtocol2::openPort(const QString &portName) {
    closePort();

    m_serial->setPortName(portName);
    m_serial->setBaudRate(QSerialPort::Baud4800);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::EvenParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        return false;
    }

    m_serial->clear(QSerialPort::AllDirections);
    m_serial->setDataTerminalReady(true);
    m_serial->setRequestToSend(false);

    m_state = CommandState::Idle;
    m_rxBuffer.clear();
    m_consecutiveTimeouts = 0;

    // Opening a USB Arduino usually resets the board, so give the sketch time to start.
    QTimer::singleShot(1500, this, [this]() {
        if (m_serial->isOpen() && !m_pollTimer->isActive()) {
            m_pollTimer->start(200);
            sendNextCommand();
        }
    });

    return true;
}

void MassaKProtocol2::closePort() {
    m_pollTimer->stop();
    m_responseTimer->stop();
    if (m_serial->isOpen()) {
        m_serial->close();
    }
    m_state = CommandState::Idle;
    m_rxBuffer.clear();
}

void MassaKProtocol2::tryAutoConnect() {
    if (m_serial->isOpen()) {
        return;
    }

    struct Candidate {
        QString portName;
        int score;
    };

    QList<Candidate> candidates;
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        int score = 0;
        const QString haystack = QString("%1 %2 %3")
                                     .arg(port.description(), port.manufacturer(), port.systemLocation())
                                     .toLower();

        if (haystack.contains("arduino")) score += 100;
        if (haystack.contains("ch340") || haystack.contains("ch341")) score += 80;
        if (haystack.contains("usb-serial") || haystack.contains("usb serial")) score += 60;
        if (haystack.contains("cp210") || haystack.contains("ftdi")) score += 50;
        if (port.hasVendorIdentifier()) score += 10;

        candidates.push_back({port.portName(), score});
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate &left, const Candidate &right) {
        return left.score > right.score;
    });

    for (const Candidate &candidate : candidates) {
        if (openPort(candidate.portName)) {
            return;
        }
    }

    emit errorOccurred("Весы не найдены: подключите Arduino и дождитесь появления COM-порта");
    if (m_autoConnect) {
        m_reconnectTimer->start();
    }
}

void MassaKProtocol2::sendNextCommand() {
    if (!m_serial->isOpen()) {
        if (m_autoConnect && !m_reconnectTimer->isActive()) {
            m_reconnectTimer->start();
        }
        return;
    }

    if (m_state != CommandState::Idle) {
        return;
    }

    sendCommand(0x44, CommandState::WaitingStatus);
}

void MassaKProtocol2::sendCommand(char command, CommandState nextState) {
    m_state = nextState;
    m_rxBuffer.clear();
    m_serial->write(&command, 1);
    m_serial->flush();
    m_responseTimer->start();
}

void MassaKProtocol2::handleReadyRead() {
    m_rxBuffer.append(m_serial->readAll());

    while (m_rxBuffer.size() >= 2 && m_state != CommandState::Idle) {
        const QByteArray response = m_rxBuffer.left(2);
        m_rxBuffer.remove(0, 2);

        const auto lowByte = static_cast<quint8>(response.at(0));
        const auto highByte = static_cast<quint8>(response.at(1));
        m_responseTimer->stop();
        m_consecutiveTimeouts = 0;

        if (m_state == CommandState::WaitingStatus) {
            m_currentData.isTareActive = ((lowByte & 0x20) != 0);
            sendCommand(0x45, CommandState::WaitingWeight);
            continue;
        }

        const bool isNegative = ((highByte & 0x80) != 0);
        const quint16 rawWeight = (static_cast<quint16>(highByte & 0x7F) << 8) | lowByte;

        double weightInKg = static_cast<double>(rawWeight) / 1000.0;
        if (isNegative) {
            weightInKg = -weightInKg;
        }

        m_currentData.weight = weightInKg;
        m_currentData.unit = QStringLiteral("кг");

        emit dataReceived(m_currentData);
        m_state = CommandState::Idle;
    }
}

void MassaKProtocol2::handleResponseTimeout() {
    m_state = CommandState::Idle;
    m_rxBuffer.clear();

    if (++m_consecutiveTimeouts < 5) {
        return;
    }

    const QString portName = m_serial->portName();
    closePort();
    emit errorOccurred(QString("Нет ответа от весов на порту %1").arg(portName));

    if (m_autoConnect) {
        m_reconnectTimer->start();
    }
}

void MassaKProtocol2::requestData() {
    sendNextCommand();
}

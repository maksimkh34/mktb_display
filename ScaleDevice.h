#pragma once
#include <QObject>

struct ScaleData {
    double weight = 0.0;
    bool isTareActive = false;
    QString unit = "кг";
};

class ScaleDevice : public QObject {
    Q_OBJECT
public:
    explicit ScaleDevice(QObject *parent = nullptr) : QObject(parent) {}
    ~ScaleDevice() override = default;

    virtual void connectDevice(const QString &portName) = 0;
    virtual void disconnectDevice() = 0;
    virtual void requestData() = 0; // Метод циклического опроса

    signals:
        void dataReceived(const ScaleData &data);
    void errorOccurred(const QString &error);
};
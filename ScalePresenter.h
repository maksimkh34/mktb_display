#pragma once
#include <QObject>
#include "ScaleView.h"
#include "ScaleDevice.h"

class ScalePresenter : public QObject {
    Q_OBJECT
public:
    ScalePresenter(ScaleView *view, ScaleDevice *device, QObject *parent = nullptr);
    ~ScalePresenter() override = default;

private slots:
    void onDataReceived(const ScaleData &data);
    void onErrorOccurred(const QString &error);

private:
    ScaleView *m_view;
    ScaleDevice *m_device;
};
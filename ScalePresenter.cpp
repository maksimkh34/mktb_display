#include "ScalePresenter.h"

ScalePresenter::ScalePresenter(ScaleView *view, ScaleDevice *device, QObject *parent)
    : QObject(parent), m_view(view), m_device(device) {
    
    connect(m_device, &ScaleDevice::dataReceived, this, &ScalePresenter::onDataReceived);
    connect(m_device, &ScaleDevice::errorOccurred, this, &ScalePresenter::onErrorOccurred);
}

void ScalePresenter::onDataReceived(const ScaleData &data) {
    m_view->updateWeight(data.weight);
    m_view->updateUnit(data.unit);
    m_view->updateTareState(data.isTareActive);
}

void ScalePresenter::onErrorOccurred(const QString &error) {
    m_view->displayError(error);
}
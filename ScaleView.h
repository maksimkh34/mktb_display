#pragma once
#include <QString>

class ScaleView {
public:
    virtual ~ScaleView() = default;
    virtual void updateWeight(double weight) = 0;
    virtual void updateUnit(const QString &unit) = 0;
    virtual void updateTareState(bool isActive) = 0;
    virtual void displayError(const QString &error) = 0;
};
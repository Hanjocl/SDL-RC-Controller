#ifndef QMLCONTROLLERAPI_H
#define QMLCONTROLLERAPI_H

#include <vector>
#include <iostream>
#include <QObject>
#include <QTimer>
#include <QVariant>

#include <SDL_keycode.h>

#include "inputController.h"

class QmlControllerApi : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList channelValues READ channelValues NOTIFY channelValuesChanged)

public:
    explicit QmlControllerApi(Inputs& controller, QObject *parent = nullptr);
    ~QmlControllerApi();

    // Polling functions
    Q_INVOKABLE void startPolling(int intervalHz = 50);
    Q_INVOKABLE void setPollingInterval(int intervalHz);
    Q_INVOKABLE void stopPolling();

    // CHANNELS
    QVariantList channelValues() const;
    
    Q_INVOKABLE void stopScanning() { scanning = false; }
    Q_INVOKABLE void injectKey(int qtKey, const QString& text);
    void setDebug(bool state) { debug = state; }

signals:
    void channelValuesChanged();

private:
    // Library specific
    Inputs &SdlController;
    void updateInputs();
    std::vector<ChannelDataType> m_channels;
    
    // Input Detection
    bool scanning = true;
    bool debug = false;
    
    // QML Timer
    QTimer m_timer;

    // Debugging
    void printChannels(const std::vector<ChannelDataType>& channels);
};

#endif // QMLCONTROLLERAPI_H
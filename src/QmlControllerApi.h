#ifndef QMLCONTROLLERAPI_H
#define QMLCONTROLLERAPI_H

#include <vector>
#include <iostream>
#include <QObject>
#include <QTimer>
#include <QVariant>

#include <SDL_keycode.h>

#include "inputController.h"
#include "ChannelConfig.h"

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
    
    // Input Detection
    Q_INVOKABLE QString setInput(int channelIndex);
    Q_INVOKABLE void stopScanning() { scanning = false; }

    // Apply config to memory
    Q_INVOKABLE bool ApplyChannelSettings(int channelIndex, int mode, int offset);
    Q_INVOKABLE bool ClearChannelConfig(int channel_index);

    //  Save and Load config file
    Q_INVOKABLE bool saveConfig(const QString& filePath = QString());
    Q_INVOKABLE bool loadConfig(const QString& filePath = QString());

    // QML to SDL Injection
    Q_INVOKABLE void injectKey(int qtKey, const QString& text);
    void setDebug(bool state) { debug = state; }

    Q_INVOKABLE std::vector<ChannelConfig> getChannelConfigs() const { return m_channel_config; }
    Q_INVOKABLE QString getChannelInputLabel(int index) const;
    Q_INVOKABLE int getMode(int channelIndex) const;
    Q_INVOKABLE int getChannelOffset(int channelIndex) const;

    
signals:
    void channelValuesChanged();
    void configLoaded();
    
private:
    // Library specific
    Inputs &SdlController;
    void updateInputs();
    std::vector<ChannelDataType> m_channels;
    std::vector<ChannelConfig> m_channel_config;
    
    // Input Detection
    bool scanning = false;
    int m_intervalHz = 50;
    
    // QML specific
    QTimer m_timer;
    QString inputLabelFromChannel(const ChannelConfig& channel) const;
    
    // Debugging
    bool debug = false;
    void printChannels(const std::vector<ChannelDataType>& channels);
};

#endif // QMLCONTROLLERAPI_H
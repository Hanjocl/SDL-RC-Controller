#ifndef QMLCONTROLLERAPI_H
#define QMLCONTROLLERAPI_H

#include <vector>
#include <iostream>
#include <QObject>
#include <QTimer>
#include <QVariant>
#include <functional>
#include <QCoreApplication>
#include <SDL_Keycode.h>

#include "inputController.h"
#include "ChannelConfig.h"


#define SDL_CONFIG_FILE_NAME "config_sdlController.json"
#define SDL_CONFIG_FILE_PATH QString(QCoreApplication::applicationDirPath() + "/" + SDL_CONFIG_FILE_NAME)

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
    Q_INVOKABLE QString getInput(int channelIndex);
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

    // Callback for sending channel outputs to other components
    void setChannelsCallback(std::function<void(const std::vector<ChannelDataType>&)> cb) {
        channels_callback = std::move(cb);
    }

    
signals:
    void channelValuesChanged();
    void configLoaded();
    
private:
    // Library specific
    Inputs &SdlController;
    void updateInputs();
    std::vector<ChannelDataType> m_channels;
    int const default_channel_value = 1500; // Should be moved to next iteration on input library...
    std::vector<ChannelConfig> m_channel_config;
    bool ApplyInputChannel(int channelIndex);
    
    // Input Detection
    bool scanning = false;
    int m_intervalHz = 50;
    
    // QML specific
    QTimer m_timer;
    QString inputLabelFromChannel(const ChannelConfig& channel) const;

    // Callback
    std::function<void(const std::vector<ChannelDataType>&)> channels_callback;

    // Debugging
    // Outputs channel values to console
    bool debug = false;
    void printChannels(const std::vector<ChannelDataType>& channels);
};

#endif // QMLCONTROLLERAPI_H
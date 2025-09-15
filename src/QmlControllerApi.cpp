#include "QmlControllerApi.h"

#include <QVariantList>
#include <iostream>
#include <SDL.h>

// QJSON for config save/load
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "JsonHelper.h"

QmlControllerApi::QmlControllerApi(Inputs& controller, QObject *parent) 
    : QObject(parent), SdlController(controller), m_channels(controller.getChannels().size()), m_channel_config(controller.getChannels().size()) {
    std::cout << "SDL Controller API: Initialized " << std::endl;
    connect(&m_timer, &QTimer::timeout, this, &QmlControllerApi::updateInputs);
}

QmlControllerApi::~QmlControllerApi() {
    std::cout << "SDL Controller API: Stopped" << std::endl;
    stopPolling();
}

void QmlControllerApi::updateInputs() {
    SdlController.cycle(m_channels);
    if (debug) {
        printChannels(m_channels);
    }
    

    const auto channels = SdlController.getChannels();

    emit channelValuesChanged(); // Notify QML to refresh the ListView
}

void QmlControllerApi::startPolling(int intervalHz) {
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    m_intervalHz = intervalHz;
    int intervalMs = 1000 / intervalHz;

    if (!m_timer.isActive()) {
        m_timer.start(intervalMs);
    } else {
        m_timer.setInterval(intervalMs);
    }
}

void QmlControllerApi::setPollingInterval(int intervalHz) {
    m_intervalHz = intervalHz;
    int intervalMs = 1000 / intervalHz;

    if (m_timer.isActive())
        m_timer.setInterval(intervalMs);
}

void QmlControllerApi::stopPolling() {
    m_timer.stop();
}

QVariantList QmlControllerApi::channelValues() const {
    QVariantList list;
    for (const auto& val : SdlController.getChannels()) {
        list.append(QVariant::fromValue(val));  // assuming ChannelDataType can be converted to QVariant
    }
    return list;
}

QString QmlControllerApi::setInput(int channelIndex) {
    ChannelConfig& channel = m_channel_config[channelIndex];
    scanning = true;

    bool wasPolling = m_timer.isActive();
    if (wasPolling) m_timer.stop();

    SDL_Event event;
    while (scanning) {
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                scanning = false;
                channel.type = InputType::Keyboard;
                channel.raw_event = event;
                channel.input_data = event.key.keysym.sym;
                channel.mode = ChannelModes::HOLD;

                return inputLabelFromChannel(channel);
            }
            else if (event.type == SDL_JOYBUTTONDOWN) {
                scanning = false;
                channel.type = InputType::JoystickButton;
                channel.raw_event = event;
                channel.input_data = JoystickButton{ event.jbutton.button, event.jbutton.which };
                channel.mode = ChannelModes::HOLD;

                return inputLabelFromChannel(channel);
            }
            else if (event.type == SDL_JOYAXISMOTION && std::abs(event.jaxis.value) > 16000) {
                scanning = false;
                channel.type = InputType::JoystickAxis;
                channel.raw_event = event;
                channel.input_data = JoystickAxis{ event.jaxis.axis, event.jaxis.which };
                channel.offset = event.jaxis.value;
                channel.mode = ChannelModes::RAW;

                return inputLabelFromChannel(channel);
            }
        }
        SDL_Delay(10);
    }

    if (wasPolling) startPolling(m_intervalHz);
    return QString();
}

QString QmlControllerApi::getChannelInputLabel(int index) const {
    if (index < 0 || index >= static_cast<int>(m_channel_config.size()))
        return QString("Invalid channel");

    return inputLabelFromChannel(m_channel_config[index]);
}

int QmlControllerApi::getMode(int channelIndex) const {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(m_channel_config.size()))
        return 0; 
    return static_cast<int>(m_channel_config[channelIndex].mode);
}

int QmlControllerApi::getChannelOffset(int channelIndex) const {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(m_channel_config.size()))
        return 0; // default offset
    return m_channel_config[channelIndex].offset;
}

QString QmlControllerApi::inputLabelFromChannel(const ChannelConfig &channel) const {
    switch (channel.type) {
        case InputType::Keyboard:
            if (std::holds_alternative<SDL_Keycode>(channel.input_data)) {
                SDL_Keycode key = std::get<SDL_Keycode>(channel.input_data);
                return QString(SDL_GetKeyName(key));
            }
            return QString("Unknown Key");

        case InputType::JoystickButton:
            if (std::holds_alternative<JoystickButton>(channel.input_data)) {
                JoystickButton btn = std::get<JoystickButton>(channel.input_data);
                return QString("Joystick %1 Button %2").arg(btn.joystick_id).arg(btn.button);
            }
            return QString("Unknown Joystick Button");

        case InputType::JoystickAxis:
            if (std::holds_alternative<JoystickAxis>(channel.input_data)) {
                JoystickAxis axis = std::get<JoystickAxis>(channel.input_data);
                return QString("Joystick %1 Axis %2").arg(axis.joystick_id).arg(axis.axis);
            }
            return QString("Unknown Joystick Axis");

        case InputType::None:
        default:
            return QString("");
    }
}

bool QmlControllerApi::ApplyChannelSettings(int channelIndex, int mode, int offset) {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(m_channel_config.size())) {
        qWarning() << "Invalid channel index:" << channelIndex;
        return false;
    }

    ChannelConfig& channel = m_channel_config[channelIndex];

    channel.mode = static_cast<ChannelModes>(mode);
    channel.offset = offset;

    emit channelValuesChanged(); // notify QML

    qDebug() << "Applied mode" << mode << "and offset" << offset << "to channel" << channelIndex;
    return true;
}

bool QmlControllerApi::ClearChannelConfig(int channelIndex) {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(m_channel_config.size())) {
        qWarning() << "Invalid channel index:" << channelIndex;
        return false;
    }

    ChannelConfig& channel = m_channel_config[channelIndex];

    // Reset all values
    channel.channel = -1;
    channel.type = InputType::None;
    channel.raw_event = SDL_Event();        // default SDL_Event
    channel.input_data = ChannelConfig::InputVariant{}; // reset std::variant
    channel.offset = 0;
    channel.mode = ChannelModes::NONE;

    emit channelValuesChanged(); // notify QML

    qDebug() << "Cleared config for channel" << channelIndex;
    return true;
}

void QmlControllerApi::injectKey(int qtKey, const QString& text) {
    // inject to SDL
    SDL_Event sdlEvent{};
    sdlEvent.type = SDL_KEYDOWN;
    sdlEvent.key.state = SDL_PRESSED;
    sdlEvent.key.repeat = 0;
    
    // Convert text to SDL key symbol (very basic mapping)
    SDL_Keycode keycode = SDL_GetKeyFromName(text.toUtf8().constData());
    if (keycode == SDLK_UNKNOWN)
    keycode = static_cast<SDL_Keycode>(qtKey);  // fallback

    sdlEvent.key.keysym.sym = keycode;
    qDebug() << "Injecting key:" << text << "with SDL keycode:" << keycode;
    SDL_PushEvent(&sdlEvent);
}

bool QmlControllerApi::saveConfig(const QString& filePath) {
    if (filePath.isEmpty()) {
        // No path given → save to default app folder
        return saveChannelConfigsDefault(m_channel_config);
    } else {
        return saveChannelConfigs(filePath, m_channel_config);
    }

    std::cout << "SDL Controller API: Config saved to " << (filePath.isEmpty() ? "default path" : filePath.toStdString()) << std::endl;
}

bool QmlControllerApi::loadConfig(const QString& filePath) {
    bool success;
    if (filePath.isEmpty()) {
        // No path given → load from default app folder
        success = loadChannelConfigsDefault(m_channel_config);
    } else {
        success = loadChannelConfigs(filePath, m_channel_config);
    }

    if (success) {
        emit configLoaded();
        emit channelValuesChanged(); // notify QML
    }
    std::cout << "SDL Controller API: Config loaded from " << (filePath.isEmpty() ? "default path" : filePath.toStdString()) << std::endl;
    return success;
}

// DEBUGGING
void QmlControllerApi::printChannels(const std::vector<ChannelDataType>& channels) {
    std::cout << "SDL Controller API: " << std::dec;
    for (size_t i = 0; i < channels.size(); ++i) {
        std::cout << channels[i];
        if (i != channels.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
}
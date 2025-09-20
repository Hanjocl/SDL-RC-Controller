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
    for (size_t i = 0; i < m_channel_config.size(); ++i) {
        m_channel_config[i].channel = static_cast<int>(i);
    }

    loadConfig();
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
    
    
    if (channels_callback) {
        channels_callback(m_channels); // call the callback
    }
    
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
    for (const auto& val : m_channels) {
        list.append(QVariant::fromValue(val));  // assuming ChannelDataType can be converted to QVariant
    }
    return list;
}

QString QmlControllerApi::getInput(int channelIndex) {
    ChannelConfig& channel = m_channel_config[channelIndex];
    scanning = true;

    bool wasPolling = m_timer.isActive();
    if (wasPolling) m_timer.stop();

    QString label = "";

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

                label =  inputLabelFromChannel(channel);
                break;
            }
            else if (event.type == SDL_JOYBUTTONDOWN) {
                scanning = false;
                channel.type = InputType::JoystickButton;
                channel.raw_event = event;
                channel.input_data = JoystickButton{ event.jbutton.button, event.jbutton.which };
                channel.mode = ChannelModes::HOLD;

                label =  inputLabelFromChannel(channel);
                break;
            }
            else if (event.type == SDL_JOYAXISMOTION && std::abs(event.jaxis.value) > 16000) {
                scanning = false;
                channel.type = InputType::JoystickAxis;
                channel.raw_event = event;
                channel.input_data = JoystickAxis{ event.jaxis.axis, event.jaxis.which };
                channel.offset = event.jaxis.value;
                channel.mode = ChannelModes::RAW;

                label = inputLabelFromChannel(channel);
                break;
            }
        }
        SDL_Delay(10);
    }

    if (wasPolling) startPolling(m_intervalHz);
    return QString(label);
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
        qWarning() << "SDL Controller API: Invalid channel index:" << channelIndex;
        return false;
    }

    ChannelConfig& channel = m_channel_config[channelIndex];

    channel.mode = static_cast<ChannelModes>(mode);
    channel.offset = offset;

    ApplyInputChannel(channelIndex);
    emit channelValuesChanged(); // notify QML

    return true;
}

bool QmlControllerApi::ClearChannelConfig(int channelIndex) {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(m_channel_config.size())) {
        qWarning() << "SDL Controller API: Invalid channel index:" << channelIndex;
        return false;
    }

    ChannelConfig& channel = m_channel_config[channelIndex];

    // Reset all values
    channel.type = InputType::None;
    channel.raw_event = SDL_Event();        // default SDL_Event
    channel.input_data = ChannelConfig::InputVariant{}; // reset std::variant
    channel.offset = 0;
    channel.mode = ChannelModes::NONE;

    ApplyInputChannel(channelIndex);

    emit channelValuesChanged(); // notify QML

    qDebug() << "SDL Controller API: Cleared config for channel" << channelIndex;
    return true;
}

bool QmlControllerApi::ApplyInputChannel(int channelIndex) {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(m_channel_config.size())) {
        qWarning() << "SDL Controller API: Invalid channel index:" << channelIndex;
        return false;
    }

    const ChannelConfig& config = m_channel_config[channelIndex];
    std::cout << "SDL Controller API: Applying config to channel " << channelIndex << ": "
              << "Type=" << static_cast<int>(config.type) << ", "
              << "Mode=" << static_cast<int>(config.mode) << ", "
              << "Offset=" << config.offset << ", ";
    // Apply input using Inputs methods
    switch (config.type) {
        case InputType::Keyboard: {
            const SDL_Keycode& key = config.raw_event.key.keysym.sym;
            std::cout << "Key= " << SDL_GetKeyName(key) << std::endl;
            switch (config.mode) {
                case ChannelModes::RAW:           SdlController.add(config.channel, key, config.offset); break;
                case ChannelModes::TAP:           SdlController.addTap(config.channel, key, config.offset); break;
                case ChannelModes::HOLD:          SdlController.addHold(config.channel, key, config.offset); break;
                case ChannelModes::RELEASE:       SdlController.addRelease(config.channel, key, config.offset); break;
                case ChannelModes::INCREMENT:     SdlController.addIncrement(config.channel, key, config.offset); break;
                case ChannelModes::TOGGLE:        SdlController.addToggle(config.channel, key, config.offset); break;
                case ChannelModes::TOGGLE_SYMETRIC:SdlController.addToggleSymmetric(config.channel, key, config.offset); break;
                default: break;
            }
            break;
        }
        case InputType::JoystickButton: {
            auto jb = std::get<JoystickButton>(config.input_data);
            std::cout << "Button= " << static_cast<int>(jb.button) << " on Joystick " << jb.joystick_id << std::endl;
            switch (config.mode) {
                case ChannelModes::TAP:           SdlController.addTap(config.channel, jb.button, jb.joystick_id, config.offset); break;
                case ChannelModes::HOLD:          SdlController.addHold(config.channel, jb.button, jb.joystick_id, config.offset); break;
                case ChannelModes::RELEASE:       SdlController.addRelease(config.channel, jb.button, jb.joystick_id, config.offset); break;
                case ChannelModes::INCREMENT:     SdlController.addIncrement(config.channel, jb.button, jb.joystick_id, config.offset); break;
                case ChannelModes::TOGGLE:        SdlController.addToggle(config.channel, jb.button, jb.joystick_id, config.offset); break;
                case ChannelModes::TOGGLE_SYMETRIC:SdlController.addToggleSymmetric(config.channel, jb.button, jb.joystick_id, config.offset); break;
                default: break;
            }
            break;
        }
        case InputType::JoystickAxis: {
            auto ja = std::get<JoystickAxis>(config.input_data);
            std::cout << "Axis= " << static_cast<int>(ja.axis) << " on Joystick " << ja.joystick_id << std::endl;
            switch (config.mode) {
                case ChannelModes::RAW:           SdlController.addAxis(config.channel, ja.axis, ja.joystick_id, config.offset); break;
                case ChannelModes::TAP:           SdlController.addAxisTap(config.channel, ja.axis, ja.joystick_id, config.offset); break;
                case ChannelModes::HOLD:          SdlController.addAxisHold(config.channel, ja.axis, ja.joystick_id, config.offset); break;
                case ChannelModes::RELEASE:       SdlController.addAxisRelease(config.channel, ja.axis, ja.joystick_id, config.offset); break;
                case ChannelModes::INCREMENT:     SdlController.addAxisIncrement(config.channel, ja.axis, ja.joystick_id, config.offset); break;
                case ChannelModes::TOGGLE:        SdlController.addAxisToggle(config.channel, ja.axis, ja.joystick_id, config.offset); break;
                case ChannelModes::TOGGLE_SYMETRIC:SdlController.addAxisToggleSymmetric(config.channel, ja.axis, ja.joystick_id, config.offset); break;
                default: break;
            }
            break;
        }
        case InputType::None: // Reset channel to defaults
            m_channels[channelIndex] = default_channel_value;
            SdlController.clear(channelIndex);
        default:
            break;
    }

    emit channelValuesChanged(); // notify QML
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
    SDL_PushEvent(&sdlEvent);
}

bool QmlControllerApi::saveConfig(const QString& filePath) {
    if (filePath.isEmpty()) {
        // No path given → save to default app folder
        return saveChannelConfigs(SDL_CONFIG_FILE_NAME ,m_channel_config);
    } else {
        return saveChannelConfigs(filePath, m_channel_config);
    }

    std::cout << "SDL Controller API: Config saved to " << (filePath.isEmpty() ? "default path" : filePath.toStdString()) << std::endl;
}

bool QmlControllerApi::loadConfig(const QString& filePath) {
    bool success;
    if (filePath.isEmpty()) {
        // No path given → load from default app folder
        success = loadChannelConfigs(SDL_CONFIG_FILE_NAME, m_channel_config);
    } else {
        success = loadChannelConfigs(filePath, m_channel_config);
    }
    // Check if load was succesfull
    if (!success) { 
        std::cout << "SDL Controller API: Config not found at (" << SDL_CONFIG_FILE_NAME << ")" << std::endl; 
        return success; 
    }    

    // Re-apply all channel configs
    for (size_t i = 0; i < m_channel_config.size(); ++i) {
        if (m_channel_config[i].type == InputType::None) continue;
        SdlController.clear(static_cast<int>(i));
        m_channels[i] = default_channel_value;
        ApplyInputChannel(static_cast<int>(i));
    }
    
    if (success) {
        emit configLoaded();
        emit channelValuesChanged();
        std::cout << "SDL Controller API: Config loaded from " << (filePath.isEmpty() ? "default path" : filePath.toStdString()) << std::endl;
    }
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
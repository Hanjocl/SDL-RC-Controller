#pragma once
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <iostream>
#include "ChannelConfig.h"

// Convert ChannelConfig -> QJsonObject
inline QJsonObject channelConfigToJson(const ChannelConfig& cfg) {
    QJsonObject obj;
    obj["channel"] = cfg.channel;
    obj["type"] = static_cast<int>(cfg.type);
    obj["mode"] = static_cast<int>(cfg.mode);
    obj["offset"] = cfg.offset;

    if (std::holds_alternative<SDL_Keycode>(cfg.input_data)) {
        SDL_Keycode kc = std::get<SDL_Keycode>(cfg.input_data);
        obj["input_type"] = "keyboard";
        obj["keycode"] = QString::fromUtf8(SDL_GetKeyName(kc));  // save as string
    } else if (std::holds_alternative<JoystickButton>(cfg.input_data)) {
        const auto& jb = std::get<JoystickButton>(cfg.input_data);
        obj["input_type"] = "joystick_button";
        obj["button"] = jb.button;
        obj["joystick_id"] = jb.joystick_id;
    } else if (std::holds_alternative<JoystickAxis>(cfg.input_data)) {
        const auto& ja = std::get<JoystickAxis>(cfg.input_data);
        obj["input_type"] = "joystick_axis";
        obj["axis"] = ja.axis;
        obj["joystick_id"] = ja.joystick_id;
    } else {
        obj["input_type"] = "none";
    }

    return obj;
}

// Convert QJsonObject -> ChannelConfig
inline ChannelConfig channelConfigFromJson(const QJsonObject& obj) {
    ChannelConfig cfg;
    cfg.channel = obj["channel"].toInt(-1);
    cfg.type = static_cast<InputType>(obj["type"].toInt(static_cast<int>(InputType::None)));
    cfg.mode = static_cast<ChannelModes>(obj["mode"].toInt(static_cast<int>(ChannelModes::NONE)));
    cfg.offset = obj["offset"].toInt(0);

    QString inputType = obj["input_type"].toString();
    if (inputType == "keyboard") {
        QString keyName = obj["keycode"].toString();
        SDL_Keycode kc = SDL_GetKeyFromName(keyName.toUtf8().constData());
        cfg.input_data = kc;

        // restore raw_event
        SDL_Event ev{};
        ev.type = SDL_KEYDOWN;  // choose KEYDOWN as default
        ev.key.keysym.sym = kc;
        cfg.raw_event = ev;

    } else if (inputType == "joystick_button") {
        JoystickButton jb;
        jb.button = static_cast<Uint8>(obj["button"].toInt());
        jb.joystick_id = static_cast<SDL_JoystickID>(obj["joystick_id"].toInt());
        cfg.input_data = jb;

        // restore raw_event
        SDL_Event ev{};
        ev.type = SDL_JOYBUTTONDOWN;
        ev.jbutton.button = jb.button;
        ev.jbutton.which = jb.joystick_id;
        cfg.raw_event = ev;

    } else if (inputType == "joystick_axis") {
        JoystickAxis ja; 
        ja.axis = static_cast<Uint8>(obj["axis"].toInt());
        ja.joystick_id = static_cast<SDL_JoystickID>(obj["joystick_id"].toInt());
        cfg.input_data = ja;

        // restore raw_event
        SDL_Event ev{};
        ev.type = SDL_JOYAXISMOTION;
        ev.jaxis.axis = ja.axis;
        ev.jaxis.which = ja.joystick_id;
        cfg.raw_event = ev;

    } else {
        cfg.input_data = std::monostate{};
        cfg.raw_event = SDL_Event{};  // clear
    }

    return cfg;
}


// Save vector<ChannelConfig> to JSON file
inline bool saveChannelConfigs(const QString& filePath, const std::vector<ChannelConfig>& configs) {
    QJsonArray arr;
    for (const auto& cfg : configs)
        arr.append(channelConfigToJson(cfg));

    QJsonDocument doc(arr);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(doc.toJson());
    return true;
}

// Load vector<ChannelConfig> from JSON file
inline bool loadChannelConfigs(const QString& filePath, std::vector<ChannelConfig>& configs) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray())
        return false;

    QJsonArray arr = doc.array();
    configs.clear();
    for (const auto& v : arr) {
        if (!v.isObject()) continue;
        configs.push_back(channelConfigFromJson(v.toObject()));
    }
    return true;
}

inline QString defaultConfigPath() {
    return QCoreApplication::applicationDirPath() + "/config.json";
}

// Save to default app folder
inline bool saveChannelConfigsDefault(const std::vector<ChannelConfig>& configs) {
    return saveChannelConfigs(defaultConfigPath(), configs);
}

// Load from default app folder
inline bool loadChannelConfigsDefault(std::vector<ChannelConfig>& configs) {
    return loadChannelConfigs(defaultConfigPath(), configs);
}

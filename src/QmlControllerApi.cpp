#include "QmlControllerApi.h"

#include <QVariantList>
// #include <QtConcurrent>
#include <iostream>
#include <SDL.h>

QmlControllerApi::QmlControllerApi(Inputs& controller, QObject *parent) 
    : QObject(parent), SdlController(controller), m_channels(controller.getChannels().size()) {
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
    int intervalMs = 1000 / intervalHz;
    if (!m_timer.isActive()) {
        m_timer.start(intervalMs);
    } else {
        m_timer.setInterval(intervalMs);
    }
}

void QmlControllerApi::setPollingInterval(int intervalHz) {
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


// DEBUGGING
void QmlControllerApi::printChannels(const std::vector<ChannelDataType>& channels) {
    std::cout << "SDL Controller API: " << std::dec;
    for (size_t i = 0; i < channels.size(); ++i) {
        std::cout << channels[i];
        if (i != channels.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
}
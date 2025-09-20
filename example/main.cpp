#define SDL_MAIN_HANDLED

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <iostream>
#include <SDL.h>

#include "inputController.h"
#include "../src/QmlControllerApi.h"

int main(int argc, char *argv[]) {    
    // Start QML
    QGuiApplication app(argc, argv);

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cout << "SDL_InitSubSystem Error: " << SDL_GetError() << std::endl;
        return false;
    }

    Inputs sdlController(16); // Create controller with 16 channels
    QmlControllerApi inputController(sdlController);
    inputController.setDebug(false);
    inputController.setChannelsCallback([](const std::vector<ChannelDataType>& channels) {
        // Example callback function to print channel values
        // std::cout << "Channel Callback values: ";
        // for (const auto& value : channels) {
        //     std::cout << value << " ";
        // }
        // std::cout << std::endl;
    });
    
    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/SDL_RC_Controller/qml/Main.qml"));

    engine.rootContext()->setContextProperty("SdlController", &inputController);
    inputController.startPolling(50);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);
        
    //inputController.startPolling(50);
    int result_app = app.exec();

    // Do stuff after shutting down app
    SDL_Quit();

    std::cout << "Closing App... " << std::endl;
    return result_app;
}

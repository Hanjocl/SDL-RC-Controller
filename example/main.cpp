#define SDL_MAIN_HANDLED

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <iostream>
#include <SDL.h>

#include "inputControllerModel.h"
#include "ControllerModel/controllerModel.h"

#include "crsf.h"

int main(int argc, char *argv[]) {    
    // Start QML
    QGuiApplication app(argc, argv);

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cout << "SDL_InitSubSystem Error: " << SDL_GetError() << std::endl;
        return false;
    }
    InputControllerModel inputController;

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/SDL_Controller/qml/Main.qml"));
    
    engine.rootContext()->setContextProperty("InputController", &inputController);
    
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

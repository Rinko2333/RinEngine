#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QSurfaceFormat>
#include <QCoreApplication>
#include <QDir>

#include "Logger.h"
#include "SettingsManager.h"
#include "ResourceManager.h"
#include "VariableManager.h"
#include "ScriptParser.h"
#include "ScriptRunner.h"
#include "GameStateManager.h"
#include "AudioManager.h"
#include "GalleryManager.h"
#include "LocalizationManager.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("RinEngine");
    QCoreApplication::setApplicationName("RinEngine");
    QCoreApplication::setApplicationVersion("0.1.0");

    QQuickStyle::setStyle("Basic");

    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    // Register C++ singletons
    qmlRegisterSingletonInstance("RinEngine.Core", 1, 0, "Logger", Logger::instance());
    qmlRegisterSingletonInstance("RinEngine.Core", 1, 0, "Settings", SettingsManager::instance());
    qmlRegisterSingletonInstance("RinEngine.Core", 1, 0, "ResourceManager", ResourceManager::instance());
    qmlRegisterSingletonInstance("RinEngine.Core", 1, 0, "VariableManager", VariableManager::instance());
    qmlRegisterSingletonInstance("RinEngine.Core", 1, 0, "ScriptParser", ScriptParser::instance());
    qmlRegisterSingletonInstance("RinEngine.Core", 1, 0, "ScriptRunner", ScriptRunner::instance());
    qmlRegisterSingletonInstance("RinEngine.Core", 1, 0, "GameStateManager", GameStateManager::instance());
    qmlRegisterSingletonInstance("RinEngine.Core", 1, 0, "AudioManager", AudioManager::instance());
    qmlRegisterSingletonInstance("RinEngine.Core", 1, 0, "GalleryManager", GalleryManager::instance());
    qmlRegisterSingletonInstance("RinEngine.Core", 1, 0, "L10n", LocalizationManager::instance());

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("RinEngine", "Main");

    return app.exec();
}

#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QByteArray>
#include <QDebug>

#include <viscaudpcontroller.h>
#include <viscaemulator.h>

#include "cutemqttclient.h"
#include "videorouterclient.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    qmlRegisterType<ViscaUdpController>("org.tal.visca", 1, 0, "ViscaController");
    qRegisterMetaType<ushort>();
    qRegisterMetaType<QMqttTopicName>();
    qmlRegisterType<CuteMqttClient>("org.tal.mqtt", 1, 0, "MqttClient");

    qmlRegisterType<VideoRouterClient>("org.tal.bm", 1, 0, "VideoRouterClient");

    QQmlApplicationEngine engine;

    ViscaEmulator emulator(&engine);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("visca", "Main");

    return app.exec();
}

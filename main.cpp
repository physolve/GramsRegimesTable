#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "prototablemodel.h"
#include "regime.h"
#include "regimemanager.h"
#include "visibleregimemodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qRegisterMetaType<Condition>();
    qmlRegisterUncreatableMetaObject(RegimeEnums::staticMetaObject, "com.grams.prototable", 1, 0, "RegimeState", "Error: only enums");
    qmlRegisterType<VisibleRegimeModel>("com.grams.prototable", 1, 0, "VisibleRegimeModel");

    QQmlApplicationEngine engine;
    RegimeManager regimeManager;
    qmlRegisterSingletonInstance("com.grams.prototable", 1, 0, "RegimeManager", &regimeManager);
    const QUrl url("qrc:/prototype_table/qml/Main.qml");
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "prototablemodel.h"
#include "regime.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qRegisterMetaType<Condition>();
    qmlRegisterType<ProtoTableModel>("com.grams.prototable", 1, 0, "ProtoTableModel");

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/prototype_table/qml/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
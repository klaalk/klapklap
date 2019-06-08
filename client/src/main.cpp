#include <cstdlib>
#include <deque>
#include <iostream>
#include <pthread.h>


#include <QApplication>
#include <QCommandLineParser>
#include <QtWidgets/QMainWindow>
#include "classes/client/kk_client.h"

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(client);
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("KlapKlapTeam");
    QCoreApplication::setApplicationName("KlapKlap");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(a);

    kk_client client(QUrl(QStringLiteral("wss://130.192.163.109:3002")));

    Q_UNUSED(client);
    return a.exec();
}

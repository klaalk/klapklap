#include <cstdlib>
#include <deque>
#include <iostream>
#include <pthread.h>


#include <QApplication>
#include <QCommandLineParser>
#include <QtWidgets/QMainWindow>

#include "classes/view/mainwindow.h"

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

    kk_client client(QUrl(QStringLiteral("wss://localhost:3310")));
    MainWindow view;

    // CONNECTION RESPONSE
    QObject::connect(&client, &kk_client::connectionSucceed, &view, &MainWindow::openWindow);

    // LOGIN REQUEST
    QObject::connect(&view, &MainWindow::tryLogin, &client, &kk_client::sendLoginRequest);

    // LOGIN RESPONSE
    QObject::connect(&client, &kk_client::loginSucceed, &view, &MainWindow::openEditor);


    Q_UNUSED(client);
    return a.exec();
}

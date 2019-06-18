#include <QApplication>
#include <QDesktopWidget>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QtWidgets/QMainWindow>


#include "classes/client/kk_client.h"

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(client);
    QApplication a(argc, argv);
//    kk_client client(QUrl(QStringLiteral("wss://192.168.21.50:3002")));
    kk_client client(QUrl(QStringLiteral("wss://130.192.163.109:3002")));

    Q_UNUSED(client);
    return a.exec();
}

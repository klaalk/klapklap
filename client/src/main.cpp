#include <cstdlib>
#include <deque>
#include <iostream>
#include <pthread.h>


#include <QApplication>
#include <QDesktopWidget>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QtWidgets/QMainWindow>


#include "classes/client/kk_client.h"
#include "classes/textedit/textedit.h"



int main(int argc, char* argv[])
{
//    Q_INIT_RESOURCE(client);
    QApplication a(argc, argv);
    kk_client client(QUrl(QStringLiteral("wss://localhost:3310")));


//    QCoreApplication::setOrganizationName("KlapKlapTeam");
//    QCoreApplication::setApplicationName("KlapKlap");
//    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
//    QCommandLineParser parser;
//    parser.setApplicationDescription(QCoreApplication::applicationName());
//    parser.addHelpOption();
//    parser.addVersionOption();
//    parser.addPositionalArgument("file", "The file to open.");
//    parser.process(a);

//    TextEdit mw;

//    const QRect availableGeometry = QApplication::desktop()->availableGeometry(&mw);
//    mw.resize(availableGeometry.width() / 2, (availableGeometry.height() * 2) / 3);
//    mw.move((availableGeometry.width() - mw.width()) / 2,
//            (availableGeometry.height() - mw.height()) / 2);

//    if (!mw.load(parser.positionalArguments().value(0, QLatin1String(":/example.html"))))
//        mw.fileNew();

//    mw.show();
    Q_UNUSED(client);
    return a.exec();
}

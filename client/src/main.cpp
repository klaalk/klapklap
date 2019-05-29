#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <pthread.h>


#include <QApplication>
#include <QDesktopWidget>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QtWidgets/QMainWindow>


#include "../../libs/src/classes/payload/kk_payload.h"
#include "classes/client/kk_client.h"

//#include "classes/textedit/textedit.h"

boost::asio::io_service io_service;
using boost::asio::ip::tcp;


int main(int argc, char* argv[])
{
//    try
//    {
//        tcp::resolver resolver(io_service);
//        tcp::resolver::query query("127.0.0.1", "3310");
//        tcp::resolver::iterator iterator = resolver.resolve(query);
//
//        kk_client c(io_service, iterator);
//
//        std::thread t_socket([&] { io_service.run(); c.close();});
//        std::thread t_sender([&] { while(c.get_connection_state() != connection_failed ){c.menu();};});
//
//        t_socket.join();
//        t_sender.join();
//    }
//    catch (std::exception& e)
//    {
//        std::cerr << "Exception: " << e.what() << "\n";
//    }


//    Q_INIT_RESOURCE(textedit);

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

    QMainWindow mw;

    const QRect availableGeometry = QApplication::desktop()->availableGeometry(&mw);
    mw.resize(availableGeometry.width() / 2, (availableGeometry.height() * 2) / 3);
    mw.move((availableGeometry.width() - mw.width()) / 2,
            (availableGeometry.height() - mw.height()) / 2);

//    if (!mw.load(parser.positionalArguments().value(0, QLatin1String(":/example.html"))))
//        mw.fileNew();

    mw.show();
    return a.exec();
}

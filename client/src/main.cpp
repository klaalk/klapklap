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
    kk_char_ptr uno = kk_char_ptr(new kk_char('1',"Elefante"));
    kk_char_ptr due = kk_char_ptr(new kk_char('2',"Elefante"));

    kk_char_ptr tre = kk_char_ptr(new kk_char('3',"Aquila"));
    tre->push_identifier(kk_identifier_ptr(new kk_identifier(30,"Aquila")));


   std::cout<<"remote insert 3[30]"<<std::endl;
   crdt->remote_insert(tre);
   crdt->print();
   std::cout<<"remote delete 1[30]"<<std::endl;
   crdt->remote_delete(uno,"Elefante");
   crdt->print();
   crdt->local_delete(kk_pos(0,2),kk_pos(1,0));
   std::cout<<"local delete da (0,1) a (1,0)"<<std::endl;
   crdt->print();

//    QCoreApplication::setOrganizationName("KlapKlapTeam");
//    QCoreApplication::setApplicationName("KlapKlap");
//    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

//    QCommandLineParser parser;
//    parser.setApplicationDescription(QCoreApplication::applicationName());
//    parser.addHelpOption();
//    parser.addVersionOption();
//    parser.addPositionalArgument("file", "The file to open.");
//    parser.process(a);

    kk_client client(QUrl(QStringLiteral("wss://localhost:3002")));
//    kk_client client(QUrl(QStringLiteral("wss://130.192.163.109:3002")));

    Q_UNUSED(client);
//    TextEdit editor_;
//    editor_.show();
//    editor_.insertRemoteText("tizio","ciao\n",0,0);
//    editor_.insertRemoteText("caio","sto scrivendo",0,5);
//    editor_.insertRemoteText("tizio","come va",0,0);

    return a.exec();
}

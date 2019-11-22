#include <QApplication>
#include <QPalette>
#include "classes/client/kk_client.h"

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(client);
    QApplication a(argc, argv);


//    KKClient client(QUrl(QStringLiteral("wss://localhost:3002")));
//    KKClient client(QUrl(QStringLiteral("wss://130.192.163.109:3002")));
//    a.setWindowIcon(QIcon(":/klapklap.icns"));
//    Q_UNUSED(client);


    QString crdtstr, crdtstr2;
    KKCrdt *crdt = new KKCrdt("Klaus", casuale);
    KKCharPtr uno = KKCharPtr(new KKChar('1',"Albo"));

    uno->pushIdentifier(KKIdentifierPtr(new KKIdentifier(12,"Albo")));
    uno->pushIdentifier(KKIdentifierPtr(new KKIdentifier(36,"Albo")));
    crdt->localInsert('A', KKPosition(0,0));
    crdt->localInsert('B', KKPosition(0,1));
    crdt->remoteInsert(uno);
    crdt->localInsert('\n', KKPosition(0,2));
    crdt->localInsert('C', KKPosition(1,0));
    crdt->localInsert('C', KKPosition(0,2));
    crdt->localInsert('\n', KKPosition(0,1));
    crdt->print();
    crdtstr = crdt->saveCrdt();
    qDebug()<<crdtstr;
    KKCrdt *crdt2 = new KKCrdt("Klaus", casuale);
    std::string cr = crdtstr.toUtf8().constData();
    crdt2->loadCrdt(cr);
    crdt2->print();
    crdtstr2=crdt2->saveCrdt();
    qDebug()<<crdtstr2;


    return a.exec();
}



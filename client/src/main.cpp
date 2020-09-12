#include <QApplication>
#include <QPalette>
#include "classes/client/kk_client.h"
#include "../../libs/src/classes/crdt/kk_crdt.h"
#include "../../libs/src/classes/crdt/char/kk_char.h"
#include <QtCore/QDebug>
#include <QFont>

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(client);
    QApplication a(argc, argv);
    KKClient client(QUrl(QStringLiteral("wss://localhost:3002")));
//   KKClient client(QUrl(QStringLiteral("wss://130.192.163.103:3002")));
    QApplication::setWindowIcon(QIcon(":/klapklap.icns"));
    Q_UNUSED(client)
    return QApplication::exec();

/*NON CANCELLARE
    KKCrdt *crdt1 = new KKCrdt("Albo", casuale);
    KKCrdt *crdt2 = new KKCrdt("Edo", casuale);
    KKCharPtr ChA = KKCharPtr(new KKChar('a',"Edo"));
    KKIdentifierPtr id1 = KKIdentifierPtr(new KKIdentifier(1,"Edo"));
    ChA->pushIdentifier(id1);
    ChA->setKKCharFont("Helvetica 1,2,12,1");
    KKCharPtr ChB = KKCharPtr(new KKChar('b',"Edo"));
    KKIdentifierPtr id2 = KKIdentifierPtr(new KKIdentifier(1,"Edo"));
    ChB->pushIdentifier(id2);
    ChB->setKKCharFont("Helvetica 1,2,12,1");

    KKCharPtr ChC = KKCharPtr(new KKChar('c',"Edo"));
    KKIdentifierPtr id3 = KKIdentifierPtr(new KKIdentifier(1,"Edo"));
    ChC->pushIdentifier(id3);
    ChC->setKKCharFont("Helvetica 1,2,12,1");

    crdt1->localInsert('a',KKPosition(0,0),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->localInsert('b',KKPosition(0,1),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->localInsert('c',KKPosition(0,1),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->localInsert('\n',KKPosition(0,1),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->localInsert('d',KKPosition(0,1),"Helvetica","Rosso");
   // crdt1->print();
    crdt1->localInsert('d',KKPosition(1,0),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->localInsert('d',KKPosition(1,0),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->localInsert('d',KKPosition(1,0),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->localInsert('a',KKPosition(0,0),"Helvetica","Rosso");
   // crdt1->print();
    crdt1->localInsert('b',KKPosition(0,1),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->localInsert('c',KKPosition(0,1),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->localInsert('\n',KKPosition(0,1),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->localInsert('d',KKPosition(0,1),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->localInsert('d',KKPosition(1,0),"Helvetica","Rosso");
   // crdt1->print();
    crdt1->localInsert('d',KKPosition(1,0),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->localInsert('d',KKPosition(1,0),"Helvetica","Rosso");
    //crdt1->print();
    crdt1->remoteInsert(ChA);
    //crdt1->print();
    crdt1->remoteInsert(ChB);
    //crdt1->print();
    crdt1->remoteInsert(ChC);
    //crdt1->print();

   list<KKCharPtr> changed = crdt1->changeMultipleKKCharFormat(KKPosition(0,0),KKPosition(1,2), "Sons","blu");
   for(auto i : changed){
       unsigned long a=crdt1->remoteFormatChange(i,"GIANNI","VERDE");
          qDebug()<<a;
          qDebug()<<"\n";

   }
    QString sav=crdt1->saveCrdt();
    qDebug()<<sav;
    qDebug()<<"\n";



    crdt2->loadCrdt(sav.toStdString());
    QString sav2=crdt2->saveCrdt();
    qDebug()<<sav2;
*/

}



#include <cstdlib>
#include <deque>
#include <iostream>
#include <pthread.h>


#include <QApplication>
#include <QDesktopWidget>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QtWidgets/QMainWindow>
#include "../../libs/src/classes/crdt/kk_crdt.h"


#include "classes/client/kk_client.h"
#include "classes/textedit/textedit.h"


int main(int argc, char* argv[])
{
    kk_crdt *crdt = new kk_crdt("Canguro", casuale);

    kk_char_ptr uno = kk_char_ptr(new kk_char('1',"Elefante"));
    uno->push_identifier(kk_identifier_ptr(new kk_identifier(30,"Elefante")));

    kk_char_ptr due = kk_char_ptr(new kk_char('2',"Elefante"));
    due->push_identifier(kk_identifier_ptr(new kk_identifier(16,"Elefante")));


    kk_char_ptr tre = kk_char_ptr(new kk_char('3',"Aquila"));
    tre->push_identifier(kk_identifier_ptr(new kk_identifier(30,"Aquila")));

    kk_char_ptr quattro = kk_char_ptr(new kk_char('4',"Babbuino"));
    quattro->push_identifier(kk_identifier_ptr(new kk_identifier(28,"Babbuino")));


   std::cout<<"remote insert 1[30]"<<std::endl;
   crdt->remote_insert(uno);
   crdt->print();
   std::cout<<"remote insert 2[16]"<<std::endl;
   crdt->remote_insert(due);
   crdt->print();
   std::cout<<"local insert A (0,0)"<<std::endl;
   crdt->local_insert('A', kk_pos(0,0));
   crdt->print();
   std::cout<<"local insert B (0,1)"<<std::endl;
   crdt->local_insert('B', kk_pos(0,1));
   crdt->print();
   std::cout<<"remote delete 1[30]"<<std::endl;
   crdt->remote_delete(uno,"Elefante");
   crdt->print();
   std::cout<<"remote insert 1[30]"<<std::endl;
   crdt->remote_insert(uno);
   crdt->print();
   std::cout<<"remote delete 1[30]"<<std::endl;
   crdt->remote_delete(uno,"Elefante");
   crdt->print();
   std::cout<<"local insert /n(0,2)"<<std::endl;
   crdt->local_insert('\n', kk_pos(0,2));
   crdt->print();

   std::cout<<"remote insert 1[30]"<<std::endl;
   crdt->remote_insert(uno);
   crdt->print();
   std::cout<<"remote insert 4[28]"<<std::endl;
   crdt->remote_insert(quattro);
   crdt->print();
   std::cout<<"local delete da (0,0) a (1,1)"<<std::endl;
   crdt->local_delete(kk_pos(0,0),kk_pos(1,1));

   crdt->print();
   std::cout<<"local insert C(0,2)"<<std::endl;
   crdt->local_insert('C', kk_pos(0,2));
   crdt->print();
   std::cout<<"local insert /n(0,1)"<<std::endl;
   crdt->local_insert('\n', kk_pos(0,1));
   crdt->print();
   std::cout<<"local insert D(0,1)"<<std::endl;
   crdt->local_insert('D', kk_pos(0,1));
   crdt->print();
   std::cout<<"local insert /n(0,1)"<<std::endl;
   crdt->local_insert('\n', kk_pos(0,1));
   crdt->print();
   std::cout<<"local insert E(0,1)"<<std::endl;
   crdt->local_insert('E', kk_pos(0,1));
   crdt->print();
   std::cout<<"remote insert due[16]"<<std::endl;
   crdt->remote_insert(due);
   crdt->print();
   crdt->local_delete(kk_pos(0,1),kk_pos(0,2));
   std::cout<<"local delete da (0,1) a (0,2)"<<std::endl;
   crdt->print();

    crdt->local_insert('F', kk_pos(0,1));
    std::cout<<"local insert F(0,1)"<<std::endl;
   crdt->print();
   std::cout<<"local insert G(0,0)"<<std::endl;
    crdt->local_insert('G', kk_pos(0,0));
   crdt->print();
   std::cout<<"local insert H(0,1)"<<std::endl;
    crdt->local_insert('H', kk_pos(0,1));
   crdt->print();
   std::cout<<"local insert I(0,1)"<<std::endl;
    crdt->local_insert('I', kk_pos(0,1));
   crdt->print();
   std::cout<<"local insert L(0,2)"<<std::endl;
    crdt->local_insert('L', kk_pos(0,2));
   crdt->print();
   std::cout<<"local insert M(0,2)"<<std::endl;
    crdt->local_insert('M', kk_pos(0,2));
   crdt->print();
   std::cout<<"local insert N(0,2)"<<std::endl;
    crdt->local_insert('N', kk_pos(0,2));
   crdt->print();
   std::cout<<"local delete da (0,0) a (0,3)"<<std::endl;
   crdt->local_delete(kk_pos(0,0),kk_pos(0,3));
   crdt->print();
   std::cout<<"local insert O(0,0)"<<std::endl;
    crdt->local_insert('O', kk_pos(0,0));
   crdt->print();
   std::cout<<"local insert P(0,1)"<<std::endl;
    crdt->local_insert('P', kk_pos(0,1));
   crdt->print();
   std::cout<<"local insert Q(0,1)"<<std::endl;
    crdt->local_insert('Q', kk_pos(0,1));
   crdt->print();
   std::cout<<"local insert R(0,2)"<<std::endl;
    crdt->local_insert('R', kk_pos(0,2));
   crdt->print();
   std::cout<<"local insert S(0,2)"<<std::endl;
    crdt->local_insert('S', kk_pos(0,2));
   crdt->print();
   std::cout<<"local insert T(0,2)"<<std::endl;
    crdt->local_insert('T', kk_pos(0,2));
   crdt->print();
   std::cout<<"local insert U(0,2)"<<std::endl;
   crdt->local_insert('U', kk_pos(0,0));
  crdt->print();
  std::cout<<"local insert V(0,2)"<<std::endl;
   crdt->local_insert('V', kk_pos(0,1));
  crdt->print();
  std::cout<<"local insert W(0,1)"<<std::endl;
   crdt->local_insert('W', kk_pos(0,1));
  crdt->print();
  std::cout<<"local insert X(0,2)"<<std::endl;
   crdt->local_insert('X', kk_pos(0,2));
  crdt->print();
  std::cout<<"local insert Y(0,2)"<<std::endl;
   crdt->local_insert('Y', kk_pos(0,2));
  crdt->print();
  std::cout<<"local insert Z(0,2)"<<std::endl;
   crdt->local_insert('Z', kk_pos(0,2));
  crdt->print();
  std::cout<<"remote delete 2[16]"<<std::endl;
  crdt->remote_delete(due,"Aquila");
  crdt->print();
  std::cout<<"remote delete 1[30]"<<std::endl;
  crdt->remote_delete(uno,"Elefante");
  crdt->print();
  std::cout<<"remote insert 2[30]"<<std::endl;
  crdt->remote_insert(due);
  crdt->print();
  std::cout<<"local insert A (0,0)"<<std::endl;
  crdt->local_insert('A', kk_pos(0,0));
  crdt->print();
  std::cout<<"local insert B (0,1)"<<std::endl;
  crdt->local_insert('B', kk_pos(0,1));
  crdt->print();
  std::cout<<"remote delete 1[30]"<<std::endl;
  crdt->remote_delete(uno,"Elefante");
  crdt->print();
  std::cout<<"remote insert 1[30]"<<std::endl;
  crdt->remote_insert(uno);
  crdt->print();
  std::cout<<"remote delete 1[30]"<<std::endl;
  crdt->remote_delete(uno,"Elefante");
  crdt->print();
  std::cout<<"local insert /n(0,2)"<<std::endl;
  crdt->local_insert('\n', kk_pos(0,2));
  crdt->print();
  std::cout<<"remote insert 3[30]"<<std::endl;
  crdt->remote_insert(tre);
  crdt->print();
  std::cout<<"remote insert 4[28]"<<std::endl;
  crdt->remote_insert(quattro);
  crdt->print();
//  std::cout<<"local delete da (0,0) a (1,1)"<<std::endl;
//  crdt->local_delete(kk_pos(0,0),kk_pos(1,1));

//  crdt->print();
  std::cout<<"local delete da (1,0) a (2,2)"<<std::endl;
  crdt->local_delete(kk_pos(1,0),kk_pos(2,2));

  crdt->print();
  std::cout<<"local insert C(0,2)"<<std::endl;
  crdt->local_insert('C', kk_pos(0,2));
  crdt->print();
  std::cout<<"local insert /n(0,1)"<<std::endl;
  crdt->local_insert('\n', kk_pos(0,1));
  crdt->print();
  std::cout<<"local insert D(0,1)"<<std::endl;
  crdt->local_insert('D', kk_pos(0,1));
  crdt->print();
  std::cout<<"local insert /n(0,1)"<<std::endl;
  crdt->local_insert('\n', kk_pos(0,1));
  crdt->print();
  std::cout<<"local insert E(0,1)"<<std::endl;
  crdt->local_insert('E', kk_pos(0,1));
  crdt->print();
  std::cout<<"remote insert due[16]"<<std::endl;
  crdt->remote_insert(due);
  crdt->print();
  crdt->local_delete(kk_pos(0,1),kk_pos(0,2));
  std::cout<<"local delete da (0,1) a (0,2)"<<std::endl;
  crdt->print();

   crdt->local_insert('F', kk_pos(0,1));
   std::cout<<"local insert F(0,1)"<<std::endl;
  crdt->print();
  std::cout<<"local insert G(0,0)"<<std::endl;
   crdt->local_insert('G', kk_pos(0,0));
  crdt->print();
  std::cout<<"local insert H(0,1)"<<std::endl;
   crdt->local_insert('H', kk_pos(0,1));
  crdt->print();
  std::cout<<"local insert I(0,1)"<<std::endl;
   crdt->local_insert('I', kk_pos(0,1));
  crdt->print();
  std::cout<<"local insert L(0,2)"<<std::endl;
   crdt->local_insert('L', kk_pos(0,2));
  crdt->print();
  std::cout<<"local insert M(0,2)"<<std::endl;
   crdt->local_insert('M', kk_pos(0,2));
  crdt->print();
  std::cout<<"local insert N(0,2)"<<std::endl;
   crdt->local_insert('N', kk_pos(0,2));
  crdt->print();
  std::cout<<"local delete da (0,0) a (0,3)"<<std::endl;
  crdt->local_delete(kk_pos(0,0),kk_pos(0,3));
  crdt->print();
  std::cout<<"local insert O(0,0)"<<std::endl;
   crdt->local_insert('O', kk_pos(0,0));
  crdt->print();
  std::cout<<"local insert P(0,1)"<<std::endl;
   crdt->local_insert('P', kk_pos(0,1));
  crdt->print();
  std::cout<<"local insert Q(0,1)"<<std::endl;
   crdt->local_insert('Q', kk_pos(0,1));
  crdt->print();
  std::cout<<"local insert R(0,2)"<<std::endl;
   crdt->local_insert('R', kk_pos(0,2));
  crdt->print();
  std::cout<<"local insert S(0,2)"<<std::endl;
   crdt->local_insert('S', kk_pos(0,2));
  crdt->print();
  std::cout<<"local insert T(0,2)"<<std::endl;
   crdt->local_insert('T', kk_pos(0,2));
  crdt->print();
  std::cout<<"local insert U(0,2)"<<std::endl;
  crdt->local_insert('U', kk_pos(0,0));
 crdt->print();
 std::cout<<"local insert V(0,2)"<<std::endl;
  crdt->local_insert('V', kk_pos(0,1));
 crdt->print();
 std::cout<<"local insert W(0,1)"<<std::endl;
  crdt->local_insert('W', kk_pos(0,1));
 crdt->print();
 std::cout<<"local insert X(0,2)"<<std::endl;
  crdt->local_insert('X', kk_pos(0,2));
 crdt->print();
 std::cout<<"local insert Y(0,2)"<<std::endl;
  crdt->local_insert('Y', kk_pos(0,2));
 crdt->print();
 std::cout<<"local insert Z(0,2)"<<std::endl;
  crdt->local_insert('Z', kk_pos(0,2));
 crdt->print();
 std::cout<<"local delete da (1,1) a (3,2)"<<std::endl;
  crdt->local_delete(kk_pos(1,1),kk_pos(3,2));
 crdt->print();


//    Q_INIT_RESOURCE(client);
//    QApplication a(argc, argv);
//    kk_client client(QUrl(QStringLiteral("wss://localhost:3310")));


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

////    if (!mw.load(parser.positionalArguments().value(0, QLatin1String(":/example.html"))))
////        mw.fileNew();

//    mw.show();
//    Q_UNUSED(client);
//    return a.exec();
    return 0;
}

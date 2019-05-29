//
//  main.cpp
//  Server
//
//  Created by Michele Luigi Greco on 01/05/2019.
//  Copyright © 2019 Michele Luigi Greco. All rights reserved.
//

#include <iostream>
#include "classes/db/kk_db.h"
#include <boost/chrono.hpp>
#include <boost/asio.hpp>
#include <stdlib.h>
#include <string>

//#include <QtSql>
#include "./classes/smtp/kk_smtp.h"
#include "classes/server/kk_server.h"
#include "../../libs/src/classes/crdt/kk_crdt.h"
#include "../../libs/src/classes/crdt/pos/kk_pos.h"

#include "../../libs/src/classes/crypto/crypto.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
//    Queste instaze sono valide per tutto il porgramma e devono essere dichiarate nel main
    sql::Driver *driver = get_driver_instance();
//    kk_db my_conn(driver);
    crypto my_crypto;


//    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
//
//
//    db.setHostName("130.192.163.109");
//    db.setPort(3000);
//    db.setDatabaseName("KLAPKLAP_DB");
//    db.setUserName("server");
//    db.setPassword("password");
//    bool ok = db.open();
////
//    int value;
//    QSqlQuery query1(db);
////    query1.setForwardOnly(true);
//    query1.exec("SELECT `ID`,`NAME`,`SURNAME`,`EMAIL`,`IMAGE`,`REGISTRATION_DATE`,`PASSWORD`,`PSWLEN` FROM `USERS`;");
//    while (query1.next()) {
//        cout << query1.value(1).toInt() << endl;
//    }
//
//    QGuiApplication a(argc, argv);
//
//    // This is a first demo application of the SmtpClient for Qt project
//
//    // First we need to create an SmtpClient object
//    // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)
//
//    SmtpClient smtp("smtp.gmail.com", 465, SmtpClient::SslConnection);
//
//    // We need to set the username (your email address) and the password
//    // for smtp authentification.
//
//    smtp.setUser("klapklap.assistence@gmail.com");
//    smtp.setPassword("Progetto2019");
//
//    // Now we create a MimeMessage object. This will be the email.
//
//    MimeMessage message;
//
//    message.setSender(new EmailAddress("grecomichele96@gmail.com", "Your Name"));
//    message.addRecipient(new EmailAddress("grecomichele96@gmail.com", "Recipient's Name"));
//    message.setSubject("SmtpClient for Qt - Demo");
//
//    // Now add some text to the email.
//    // First we create a MimeText object.
//
//    MimeText text;
//
//    text.setText("Hi,\nThis is a simple email message.\n");
//
//    // Now add it to the mail
//
//    message.addPart(&text);
//
//    // Now we can send the mail
//
//    smtp.connectToHost();
//    smtp.login();
//    smtp.sendMail(message);
//    smtp.quit();

    kk_smtp mail;
    QString text = mail.QSMTP_message_builder("Prova", "Michele6000", "Messaggio di prova", "Testo bottone",
                                              "http://www.facebook.it");

    mail.QSMTP_send_message(text, "Michele Luigi Greco", "grecomichele96@gmmail.com", "Test1_QTMAIL_service");


//    kk_crdt *crdt = new kk_crdt("Canguro", casuale);
//    crdt->local_insert('c', kk_pos(0, 0));
//    crdt->local_insert('a', kk_pos(0, 1));
//    crdt->local_insert('o', kk_pos(0, 2));
//    crdt->local_insert('i', kk_pos(0, 3));
//    crdt->local_insert('a', kk_pos(0, 4));
//    crdt->local_insert('a', kk_pos(0, 5));
//    crdt->local_insert('a', kk_pos(0, 1));
//    crdt->print();
    try {
        boost::asio::io_service io_service;
        // SOLO UNA PORTA APERTA
        int port = 4040;
        tcp::endpoint endpoint(tcp::v4(), port);
        kk_server_ptr server(new kk_server(io_service, endpoint, driver));
        io_service.run();
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;


}



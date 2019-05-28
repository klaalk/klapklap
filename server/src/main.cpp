//
//  main.cpp
//  Server
// prova
//  Created by Michele Luigi Greco on 01/05/2019.
//  Copyright © 2019 Michele Luigi Greco. All rights reserved.
//

#include <iostream>
#include "./classes/db_connector/db_connector.h"
#include "./classes/db_interface/db_interface.h"
#include "./classes/db_interface/HTTP_listener/HTTP_listener.h"
#include "../src/classes/SMTP_client/SMTP_client.h"
#include "./classes/crdt/crdt_server.h"

#include <QtSql>
#include "./classes/QSMTP_service/src/SmtpMime"

#include "../../libs/src/classes/crypto/crypto.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
//    Queste instaze sono valide per tutto il porgramma e devono essere dichiarate nel main
    sql::Driver *driver = get_driver_instance();
    db_connector my_conn(driver);
    crypto my_crypto;


//    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
//
//    db.setHostName("130.192.163.109");
//    db.setPort(3000);
//    db.setDatabaseName("KLAPKLAP_DB");
//    db.setUserName("server");
//    db.setPassword("password");
//    bool ok = db.open();
//
//    int value;
//    QSqlQuery query1(db);
////    query1.setForwardOnly(true);
//    query1.exec("SELECT `ID`,`NAME`,`SURNAME`,`EMAIL`,`IMAGE`,`REGISTRATION_DATE`,`PASSWORD`,`PSWLEN` FROM `USERS`;");
//    while (query1.next()) {
//        cout << query1.value(1).toInt() << endl;
//    }

    QGuiApplication a(argc, argv);

    // This is a first demo application of the SmtpClient for Qt project

    // First we need to create an SmtpClient object
    // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)

    SmtpClient smtp("smtp.gmail.com", 465, SmtpClient::SslConnection);

    // We need to set the username (your email address) and the password
    // for smtp authentification.

    smtp.setUser("klapklap.assistence@gmail.com");
    smtp.setPassword("Progetto2019");

    // Now we create a MimeMessage object. This will be the email.

    MimeMessage message;

    message.setSender(new EmailAddress("grecomichele96@gmail.com", "Your Name"));
    message.addRecipient(new EmailAddress("grecomichele96@gmail.com", "Recipient's Name"));
    message.setSubject("SmtpClient for Qt - Demo");

    // Now add some text to the email.
    // First we create a MimeText object.

    MimeText text;

    text.setText("Hi,\nThis is a simple email message.\n");

    // Now add it to the mail

    message.addPart(&text);

    // Now we can send the mail

    smtp.connectToHost();
    smtp.login();
    smtp.sendMail(message);
    smtp.quit();
    return 0;


}



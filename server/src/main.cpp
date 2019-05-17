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

#include <Poco/Crypto/CipherFactory.h>
#include <Poco/Crypto/Cipher.h>
#include <Poco/Crypto/CipherKey.h>


int main(int argc, char *argv[]) {
//    Queste instaze sono valide per tutto il porgramma e devono essere dichiarate nel main
    sql::Driver *driver = get_driver_instance();
    db_connector my_conn(driver);

    int ret;



//
    if ((ret=my_conn.db_insert_user("paola", "password", "paola.caso96@gmail.com", "Paola", "Caso")) < 0)
    cout << ret << endl;

//    my_conn.db_insert_user("Klaus", "password", "kllaus@live.it", "Klaus", "cuko");
////    cout << ret << endl;

//    if ((ret = my_conn.db_insert_file("Michele", "file23.txt", "./file23.txt")) < 0)
//        cout << ret << endl;

////    my_conn.db_insert_file("paola_dracarys","lista_gente_da_bruciare.txt","./lista_gente_da_bruciare.txt");

//
 if(my_conn.db_login("paola","fsdfhsjkf"))
    cout<<"FUNZIA! :D"<< endl;

// Test email service
//    SMTP_client sender;
//    std::string mex=sender.SMTP_message_builder("Welcome","Rocco Greco","Try now our new application!","Download Now","http://www.facebook.it");
//    sender.SMPT_sendmail(mex,"grecomichele96@gmail.com","Test5");

//    my_conn.db_share_file("paola_dracarys","michele6000","lista_gente_da_bruciare.txt");

    return 0;
}

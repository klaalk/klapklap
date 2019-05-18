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
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <cstring>

#include "../src/classes/db_connector/db_crypto.h"

int main(int argc, char *argv[]) {
//    Queste instaze sono valide per tutto il porgramma e devono essere dichiarate nel main
    sql::Driver *driver = get_driver_instance();
    db_connector my_conn(driver);
    db_crypto my_crypto;

//
//    int ret;

//
//    cout << key1 << endl ;
////
//    if ((ret=my_conn.db_insert_user("paola", "password", "paola.caso96@gmail.com", "Paola", "Caso")) < 0)
//    cout << ret << endl;

//    my_conn.db_insert_user("Klaus", "password", "kllaus@live.it", "Klaus", "cuko");
////    cout << ret << endl;

//    if ((ret = my_conn.db_insert_file("Michele", "file23.txt", "./file23.txt")) < 0)
//        cout << ret << endl;

////    my_conn.db_insert_file("paola_dracarys","lista_gente_da_bruciare.txt","./lista_gente_da_bruciare.txt");

////
// if(my_conn.db_login("paola","fsdfhsjkf"))
//    cout<<"FUNZIA! :D"<< endl;

// Test email service
//    SMTP_client sender;
//    std::string mex=sender.SMTP_message_builder("Welcome","Rocco Greco","Try now our new application!","Download Now","http://www.facebook.it");
//    sender.SMPT_sendmail(mex,"grecomichele96@gmail.com","Test5");

//    my_conn.db_share_file("paola_dracarys","michele6000","lista_gente_da_bruciare.txt");



    /* Buffer for the tag */
    unsigned char tag[16];
    int cifred_len = 0;

    std::string test = my_crypto.db_encrypt("stringa di prova", tag, &cifred_len);
    cout << "Testo Password cifrata: " << test << endl;
    test = my_crypto.db_decrypt(test, tag, &cifred_len);


    if (cifred_len < 0) {
        /* Verify error */
        printf("Decrypted text failed to verify\n");
    } else {
        /* Show the decrypted text */
        printf("Decrypted text is:\n");
        cout << test << endl;
    }

    /* Remove error strings */
    ERR_free_strings();

    return 0;


}



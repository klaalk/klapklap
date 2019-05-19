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









    return 0;


}



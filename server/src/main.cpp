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

int main(int argc, char *argv[]) {
    sql::Driver *driver = get_driver_instance();
    db_connector my_conn(driver);

    int ret;


//    if ((ret=my_conn.db_insert_user("Michela", "password", "michele@live.it", "Michele Luigi", "Greco")) < 0)
//    cout << ret << endl;
//
//    if ((ret=my_conn.db_insert_user("Klaus", "password", "kllaus@live.it", "Klaus", "cuko")) < 0)
//    cout << ret << endl;

//    if ((ret = my_conn.db_insert_file("Michele", "file23.txt", "./file23.txt")) < 0)
//        cout << ret << endl;

// Test email service
    SMTP_client sender;
    std::string mex=sender.SMTP_message_builder("Welcome","Rocco Greco","Try now our new application!","Download Now","http://www.facebook.it");
    sender.SMPT_sendmail(mex,"grecomichele96@gmail.com","Test5");

    return 0;
}

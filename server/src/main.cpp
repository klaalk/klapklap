//
//  main.cpp
//  Server
// prova
//  Created by Michele Luigi Greco on 01/05/2019.
//  Copyright © 2019 Michele Luigi Greco. All rights reserved.
//

#include <iostream>
#include <boost/chrono.hpp>
#include <boost/asio.hpp>
#include "./classes/db_connector/db_connector.h"

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;

int main(int argc, const char * argv[]) {
    db_connector connection(get_driver_instance());

    //TODO: passare numero di parametri
    connection.db_query("SELECT USERNAME FROM USERS WHERE USERNAME='Michele6000';");
    return 0;
}

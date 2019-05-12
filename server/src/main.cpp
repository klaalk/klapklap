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

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;

int main(int argc, const char * argv[]) {
    std::unique_ptr<db_interface> interface;
    interface->start();
    return EXIT_SUCCESS;
}

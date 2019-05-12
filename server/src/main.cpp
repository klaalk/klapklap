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

#include "./classes/crdt/crdt_server.h"

int main(int argc, char* argv[])
{
    try
    {
        boost::asio::io_service io_service;
        // SOLO UNA PORTA APERTA
        int port = 3310;
        tcp::endpoint endpoint(tcp::v4(), port);
        crdt_server_ptr server(new crdt_server(io_service, endpoint));
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

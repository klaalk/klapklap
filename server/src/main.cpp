//
//  main.cpp
//  Server
//
//  Created by Michele Luigi Greco on 01/05/2019.
//  Copyright © 2019 Michele Luigi Greco. All rights reserved.
//

#include <iostream>
#include <boost/chrono.hpp>
#include <boost/asio.hpp>

#include "./classes/chat/chat_server.h"

int main(int argc, char* argv[])
{
    try
    {
        boost::asio::io_service io_service;
        // SOLO UNA PORTA APERTA
        int port = 3310;
        tcp::endpoint endpoint(tcp::v4(), port);
        chat_server_ptr server(new chat_server(io_service, endpoint));
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

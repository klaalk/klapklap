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
#include <stdlib.h>
#include <string>

#include "classes/server/kk_server.h"
#include "../../libs/src/classes/crdt/kk_crdt.h"
#include "../../libs/src/classes/crdt/pos/kk_pos.h"

int main(int argc, char* argv[])
{


    kk_crdt *crdt = new kk_crdt("Canguro",casuale);
    crdt->local_insert('c',kk_pos(0,0));
    crdt->local_insert('a',kk_pos(0,1));
    crdt->local_insert('o',kk_pos(0,2));
    crdt->local_insert('i',kk_pos(0,3));
    crdt->local_insert('a',kk_pos(0,4));
    crdt->local_insert('a',kk_pos(0,5));
    crdt->local_insert('a',kk_pos(0,1));
    crdt->print();
    try
    {
        boost::asio::io_service io_service;
        // SOLO UNA PORTA APERTA
        int port = 3310;
        tcp::endpoint endpoint(tcp::v4(), port);
        kk_server_ptr server(new kk_server(io_service, endpoint));
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}

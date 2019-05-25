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

#include "./classes/crdt/crdt_server.h"
#include "./classes/crdt_rc/CRDT_Crdt.h"


int main(int argc, char* argv[])
{


    CRDT_Crdt *crdt = new CRDT_Crdt("Canguro",casuale);
    crdt->local_insert('c',CRDT_pos(0,0));
    crdt->local_insert('a',CRDT_pos(0,1));
    crdt->local_insert('o',CRDT_pos(0,2));
    crdt->local_insert('i',CRDT_pos(0,1));
    crdt->local_insert('a',CRDT_pos(0,1));
    crdt->local_insert('a',CRDT_pos(0,1));
    crdt->local_insert('a',CRDT_pos(0,1));

   /* try
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
    */
    crdt->print();
    return 0;
}

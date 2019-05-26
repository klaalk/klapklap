#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <pthread.h>
#include "../../libs/src/classes/payload/kk_payload.h"
#include "classes/client/kk_client.h"


boost::asio::io_service io_service;
using boost::asio::ip::tcp;


int main(int argc, char* argv[])
{
    try
    {
        tcp::resolver resolver(io_service);
        tcp::resolver::query query("127.0.0.1", "3310");
        tcp::resolver::iterator iterator = resolver.resolve(query);

        kk_client c(io_service, iterator);

        std::thread t_socket([&] { io_service.run(); c.close();});
        std::thread t_sender([&] { while(c.get_connection_state() != connection_failed ){c.menu();};});

        t_socket.join();
        t_sender.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
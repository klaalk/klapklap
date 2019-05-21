#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <pthread.h>
#include "../../libs/src/classes/message/message.h"
#include "./classes/chat/chat_client.h"


boost::asio::io_service io_service;

void *start(void*)
{
    io_service.run();
    return 0;
}


using boost::asio::ip::tcp;
int main(int argc, char* argv[])
{
    try
    {
        //pthread_t client_thread;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query("127.0.0.1", "3310");
        tcp::resolver::iterator iterator = resolver.resolve(query);

        chat_client c(io_service, iterator);
        io_service.run();

        //pthread_create(&client_thread, NULL, &start, NULL);


        c.close();
        //pthread_join(client_thread, NULL);
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
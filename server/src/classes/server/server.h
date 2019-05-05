//
// Created by Klaus on 03/05/2019.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H
//importing libraries
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "../connection_handler/con_handler.h"

using namespace boost::asio;
using ip::tcp;
using std::cout;
using std::endl;

class Server {
private:
    tcp::acceptor acceptor_;
    void start_accept();
public:
    /**
     * Costruttore del server.
     */
    Server(boost::asio::io_service& io_service);
    /**
     * Accetta nuove richieste di connessione.
     */
    void handle_accept(con_handler::pointer connection, const boost::system::error_code& err);
};
#endif //SERVER_SERVER_H

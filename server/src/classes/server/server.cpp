//
// Created by Augens on 03/05/2019.
//

#include "server.h"
#include "../connection_handler/con_handler.cpp"

class Server {
private:
    tcp::acceptor acceptor_;
    ///
    void start_accept()
    {
        // socket
        con_handler::pointer connection = con_handler::create(acceptor_.get_io_service());

        // asynchronous accept operation and wait for a new connection.
        acceptor_.async_accept(connection->socket(),
                               boost::bind(&Server::handle_accept, this, connection,
                                           boost::asio::placeholders::error));
    }
public:
    //constructor for accepting connection from client
    Server(boost::asio::io_service& io_service): acceptor_(io_service, tcp::endpoint(tcp::v4(), 3310))
    {
        start_accept();
    }

    void handle_accept(con_handler::pointer connection, const boost::system::error_code& err)
    {
        if (!err) {
//          qui viene gestita la connessione in entrata

            while(1) connection->start();

        }
//      dopo aver gestito mi rimetto in attesa di nuove connessioni
        start_accept();
    }
};
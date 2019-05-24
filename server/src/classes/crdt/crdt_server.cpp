//
// Created by Klaus on 06/05/2019.
//

#include "crdt_server.h"

    crdt_server::crdt_server(boost::asio::io_service& io_service,
                const tcp::endpoint& endpoint)
            : io_service_(io_service),
              acceptor_(io_service, endpoint)
    {

        crdt_session_ptr new_session(new crdt_session(io_service_, room_));
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&crdt_server::handle_accept, this, new_session,
                                           boost::asio::placeholders::error));
    }

    void crdt_server::handle_accept(crdt_session_ptr session,
                       const boost::system::error_code& error)
    {
        if (!error)
        {
            std::cout << "handle_accept" << std::endl;
            session->start();
            crdt_session_ptr new_session(new crdt_session(io_service_, room_));
            acceptor_.async_accept(new_session->socket(),
                                   boost::bind(&crdt_server::handle_accept, this, new_session,
                                               boost::asio::placeholders::error));
        }
    }
//
// Created by Klaus on 06/05/2019.
//

#include "chat_server.h"

    chat_server::chat_server(boost::asio::io_service& io_service,
                const tcp::endpoint& endpoint)
            : io_service_(io_service),
              acceptor_(io_service, endpoint)
    {
        chat_session_ptr new_session(new chat_session(io_service_, room_));
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&chat_server::handle_accept, this, new_session,
                                           boost::asio::placeholders::error));
    }

    void chat_server::handle_accept(chat_session_ptr session,
                       const boost::system::error_code& error)
    {
        if (!error)
        {
            std::cout << "handle_accept" << std::endl;
            session->start();
            chat_session_ptr new_session(new chat_session(io_service_, room_));
            acceptor_.async_accept(new_session->socket(),
                                   boost::bind(&chat_server::handle_accept, this, new_session,
                                               boost::asio::placeholders::error));
            clients.push_front(new_session);
        }
    }
//
// Created by Klaus on 06/05/2019.
//

#ifndef SERVER_CHAT_SERVER_H
#define SERVER_CHAT_SERVER_H

#include <boost/asio.hpp>
#include "./session/chat_session.h"
#include "./room/chat_room.h"

using boost::asio::ip::tcp;

class chat_server
{
public:
    chat_server(boost::asio::io_service& io_service, const tcp::endpoint& endpoint);
    void handle_accept(chat_session_ptr session, const boost::system::error_code& error);

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
    chat_room room_;
};

typedef boost::shared_ptr<chat_server> chat_server_ptr;
#endif //SERVER_CHAT_SERVER_H

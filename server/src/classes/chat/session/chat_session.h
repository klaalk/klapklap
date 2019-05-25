//
// Created by Klaus on 06/05/2019.
//

#ifndef SERVER_CHAT_SESSION_H
#define SERVER_CHAT_SESSION_H

#include "../../../../../libs/src/classes/message/message.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "../partecipant/chat_partecipant.h"
#include "../room/chat_room.h"

using boost::asio::ip::tcp;
class chat_session : public chat_participant, public boost::enable_shared_from_this<chat_session>
{
public:
    chat_session(boost::asio::io_service& io_service, chat_room& room);

    tcp::socket& socket();

    void start();

    void deliver(const message& msg);

    void handle_read_header(const boost::system::error_code& error);

    void handle_read_body(const boost::system::error_code& error);

    void handle_write(const boost::system::error_code& error);

private:
    tcp::socket socket_;
    chat_room& room_;
    message read_msg_;
    message_queue write_msgs_;
};

typedef boost::shared_ptr<chat_session> chat_session_ptr;

#endif //SERVER_CHAT_SESSION_H

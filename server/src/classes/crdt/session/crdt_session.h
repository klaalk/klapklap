//
// Created by Klaus on 06/05/2019.
//

#ifndef SERVER_CHAT_SESSION_H
#define SERVER_CHAT_SESSION_H

#include "../../../../../libs/src/classes/chat/message/chat_message.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "boost/filesystem.hpp"   // includes all needed Boost.Filesystem declarations


#include "../partecipant/crdt_partecipant.h"
#include "../file/crdt_file.h"
#include "../room/crdt_room.h"

using boost::asio::ip::tcp;

class crdt_session : public crdt_participant, public boost::enable_shared_from_this<crdt_session> {
public:
    crdt_session(boost::asio::io_service &io_service, crdt_room &room_);

    tcp::socket &socket();

    void start();

    void deliver(const chat_message &msg);

    void handle_read_header(const boost::system::error_code &error);

    void handle_read_body(const boost::system::error_code &error);

    void handle_write(const boost::system::error_code &error);

private:
    tcp::socket socket_;
    crdt_room &room_;
    crdt_file actual_file_;

    bool isInWriteMode_ = false;
    chat_message read_msg_;
    crdt_message_queue write_msgs_;
};

typedef boost::shared_ptr<crdt_session> crdt_session_ptr;

#endif //SERVER_CHAT_SESSION_H

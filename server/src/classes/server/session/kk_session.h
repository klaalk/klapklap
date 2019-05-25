//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_SESSION_H
#define KK_SESSION_H

#include "../../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../../libs/src/constants/kk_constants.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "boost/filesystem.hpp"   // includes all needed Boost.Filesystem declarations


#include "../partecipant/kk_partecipant.h"
#include "../file/kk_file.h"
#include "../room/kk_room.h"

using boost::asio::ip::tcp;

class kk_session : public kk_participant, public boost::enable_shared_from_this<kk_session>
{
public:
    kk_session(boost::asio::io_service& io_service, kk_room& room_);

    tcp::socket& socket();

    void start();

    void deliver(const kk_payload& msg);

    void handle_read_header(const boost::system::error_code& error);

    void handle_read_body(const boost::system::error_code& error);

    void handle_request();

    void handle_response(const char *body, kk_payload_type _type, kk_payload_result_type _result);

    void handle_write(const boost::system::error_code& error);

private:
    tcp::socket socket_;
    kk_room& room_;
    kk_file actual_file_;

    kk_payload read_msg_;
    kk_kk_payload_queue write_msgs_;
};

typedef boost::shared_ptr<kk_session> kk_session_ptr;

#endif //KK_SESSION_H

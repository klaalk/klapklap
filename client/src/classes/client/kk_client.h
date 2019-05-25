//
// Created by Augens on 06/05/2019.
//

#ifndef CLIENT_CHAT_CLIENT_H
#define CLIENT_CHAT_CLIENT_H


#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../libs/src/constants/kk_constants.h"

using boost::asio::ip::tcp;

typedef std::deque<kk_payload> kk_payload_queue;

class kk_client
{
public:
    kk_client(boost::asio::io_service& io_service,
                tcp::resolver::iterator endpoint_iterator);
    void write(const kk_payload& msg);
    void close();
private:
    void handle_connect(const boost::system::error_code& error,
                        tcp::resolver::iterator endpoint_iterator);

    void handle_read_header(const boost::system::error_code& error);

void handle_read_body(const boost::system::error_code& error);

    void handle_write(const boost::system::error_code& error);

    void send_kk_payload(const char *line, kk_payload_type _type);

    void receive_kk_payload();

    void menu();

    void do_login();

    void do_write(kk_payload msg);

    void do_close();
private:
    boost::asio::io_service& io_service_;
    tcp::socket socket_;
    kk_payload read_msg_;
    kk_payload_queue write_msgs_;
};


#endif //CLIENT_CHAT_CLIENT_H

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
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../../../../libs/src/classes/payload/kk_payload.h"
#include "../../../../libs/src/constants/kk_constants.h"

using boost::asio::ip::tcp;
typedef std::deque<kk_payload> kk_payload_queue;

class kk_client
{
public:
    kk_client(boost::asio::io_service& io_service,
                tcp::resolver::iterator endpoint_iterator);

    kk_client_connection_state get_connection_state();
    kk_client_state get_state();

    void send_login();
    void send_signup();
    void send_openfile();
    void send_chat();
    void send_crdt();

    void menu();
    void close();

private:

    void send_kk_payload(const char *line, kk_payload_type _type);
    void receive_kk_payload();

    void do_connection(tcp::resolver::iterator endpoint_iterator);
    void handle_connect(const boost::system::error_code& error,
                        tcp::resolver::iterator endpoint_iterator);

    void do_read();
    void handle_read_header(const boost::system::error_code& error);
    void handle_read_body(const boost::system::error_code& error);

    void do_write(kk_payload msg);
    void handle_write(const boost::system::error_code& error);

    void do_close();
    void print_chat();


    boost::asio::io_service& io_service_;
    tcp::socket socket_;

    kk_payload read_msg_;
    kk_payload_queue write_msgs_;

    kk_client_connection_state connection_state;
    kk_client_state state;

    std::condition_variable cv;
    std::mutex cv_m;

    char username[128], pass[128];
};


#endif //CLIENT_CHAT_CLIENT_H

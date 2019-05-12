//
// Created by jsnow on 12/05/19.
//

#ifndef SERVER_HTTP_LISTENER_H
#define SERVER_HTTP_LISTENER_H
#include "../../classes_include.h"
#include "../HTTP_session/HTTP_session.h"

// Accepts incoming connections and launches the sessions
class HTTP_listener : public std::enable_shared_from_this<HTTP_listener>
{
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::string const& doc_root_;
    void
    fail(boost::system::error_code ec, char const* what);
public:
    HTTP_listener(
        boost::asio::io_context& ioc,
        tcp::endpoint endpoint,
        std::string const& doc_root);

    void run();
    void do_accept();
    void on_accept(boost::system::error_code ec);
};

#endif //SERVER_HTTP_LISTENER_H

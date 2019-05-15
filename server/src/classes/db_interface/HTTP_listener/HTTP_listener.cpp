//
// Created by jsnow on 12/05/19.
//

#include "HTTP_listener.h"

HTTP_listener::HTTP_listener(
        boost::asio::io_context &ioc,
        tcp::endpoint endpoint,
        std::string const &doc_root)
        : acceptor_(ioc), socket_(ioc), doc_root_(doc_root) {
    boost::system::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        fail(ec, "open");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec) {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(
            boost::asio::socket_base::max_listen_connections, ec);
    if (ec) {
        fail(ec, "listen");
        return;
    }
}

void HTTP_listener::fail(boost::system::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

// Start accepting incoming connections
void HTTP_listener::run() {
    if (!acceptor_.is_open())
        return;
    do_accept();
}

void HTTP_listener::do_accept() {
    acceptor_.async_accept(
            socket_,
            std::bind(
                    &HTTP_listener::on_accept,
                    shared_from_this(),
                    std::placeholders::_1));
}

void HTTP_listener::on_accept(boost::system::error_code ec) {
    if (ec) {
        fail(ec, "accept");
    } else {
        // Create the session and run it
        std::make_shared<HTTP_session>(
                std::move(socket_),
                doc_root_)->run();
    }

    // Accept another connection
    do_accept();
}

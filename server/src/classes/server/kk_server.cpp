//
// Created by Klaus on 06/05/2019.
//

#include "kk_server.h"

kk_server::kk_server(boost::asio::io_service &io_service,
                     const tcp::endpoint &endpoint)
        : io_service_(io_service),
          acceptor_(io_service, endpoint) {

    kk_session_ptr new_session(new kk_session(io_service_, room_));
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&kk_server::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
}

void kk_server::handle_accept(kk_session_ptr session,
                              const boost::system::error_code &error) {
    if (!error) {
        std::cout << "handle_accept" << std::endl;
        session->start();
        kk_session_ptr new_session(new kk_session(io_service_, room_));
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&kk_server::handle_accept, this, new_session,
                                           boost::asio::placeholders::error));
    }
}
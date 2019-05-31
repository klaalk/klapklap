//
// Created by Klaus on 06/05/2019.
//

#include "kk_server.h"

kk_server::kk_server(boost::asio::io_service& io_service, unsigned short port,sql::Driver *driver)
        : io_service_(io_service),
        acceptor_(io_service,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
context_(boost::asio::ssl::context::sslv23) {

    context_.set_options(
            boost::asio::ssl::context::default_workarounds
            | boost::asio::ssl::context::no_sslv2
            | boost::asio::ssl::context::single_dh_use);
    context_.set_password_callback(boost::bind(&kk_server::get_password, this));

    context_.use_certificate_chain_file(":/localhost.cert");
    context_.use_private_key_file(":/localhost.key", boost::asio::ssl::context::pem);
//    context_.use_tmp_dh_file("/home/jsnow/Documenti/progetto_malnati/server/src/certificates/dh512.pem");

    db = std::shared_ptr<kk_db>(new kk_db(driver));
    start_accept();
}

std::string kk_server::get_password() const {
    return "winteriscoming";
}

void kk_server::start_accept() {
    kk_session_ptr new_session(new kk_session(io_service_, context_, room_, db));
    acceptor_.async_accept(new_session->socket(),
            boost::bind(&kk_server::handle_accept, this, new_session, boost::asio::placeholders::error));
}

void kk_server::handle_accept(kk_session_ptr new_session, const boost::system::error_code& error) {
    if (!error) {
        std::cout << "handle_accept" << std::endl;
        new_session->start();
    }
    else {
        delete &new_session;
    }
    start_accept();
}
//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_SERVER_H
#define KK_SERVER_H

#include <cstdlib>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>

#include "session/kk_session.h"
#include "file/kk_file.h"

using boost::asio::ip::tcp;

typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

typedef std::list<kk_session_ptr> kk_client_list;

class kk_server {
public:
    kk_server(boost::asio::io_service& io_service, unsigned short port,sql::Driver *driver);

    std::string get_password() const;

    void start_accept();

    void handle_accept(kk_session_ptr new_session,const boost::system::error_code& error);

private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ssl::context context_;

    kk_room room_;
    std::shared_ptr<kk_db> db;
};

typedef boost::shared_ptr<kk_server> kk_server_ptr;
#endif //KK_SERVER_H

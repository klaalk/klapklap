//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_SERVER_H
#define KK_SERVER_H

#include <boost/asio.hpp>
#include "session/kk_session.h"
#include "file/kk_file.h"

using boost::asio::ip::tcp;
typedef std::list<kk_session_ptr> kk_client_list;
class kk_server
{
public:
    kk_server(boost::asio::io_service &io_service, const tcp::endpoint &endpoint,sql::Driver *driver);

    void handle_accept(kk_session_ptr session, const boost::system::error_code &error);

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
    kk_room room_;
    std::shared_ptr<kk_db> db;
};

typedef boost::shared_ptr<kk_server> kk_server_ptr;
#endif //KK_SERVER_H

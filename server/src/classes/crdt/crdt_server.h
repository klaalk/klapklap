//
// Created by Klaus on 06/05/2019.
//

#ifndef SERVER_CRDT_SERVER_H
#define SERVER_CRDT_SERVER_H

#include <boost/asio.hpp>
#include "./session/crdt_session.h"
#include "./file/crdt_file.h"

using boost::asio::ip::tcp;
typedef std::list<crdt_session_ptr> crdt_client_list;
class crdt_server
{
public:
    crdt_server(boost::asio::io_service& io_service, const tcp::endpoint& endpoint);
    void handle_accept(crdt_session_ptr session, const boost::system::error_code& error);

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
    crdt_room room_;
};

typedef boost::shared_ptr<crdt_server> crdt_server_ptr;
#endif //SERVER_CRDT_SERVER_H

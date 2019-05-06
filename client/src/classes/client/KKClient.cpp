//
// Created by Klaus on 05/05/2019.
//

#include "KKClient.h"

KKClient::KKClient(boost::asio::io_service& io_service): _socket(io_service){
    //connection
    _socket.async_connect(tcp::endpoint( boost::asio::ip::address::from_string("192.168.1.8"), 3310 ),
                          boost::bind(&KKClient::handle_accept, this,
                                      boost::asio::placeholders::error));
    cout<< "Client connesso" << endl;
}

bool KKClient::send(string msg) {
    write( _socket, boost::asio::buffer(msg, msg.size()), _send_error);
    if( !_send_error ) {
        cout << "Client sent: " << msg << endl;
    }
    else {
        cout << "Send failed: " << _send_error.message() << endl;
        return false;
    }
    return true;
}

void KKClient::listen() {
    _socket.async_read_some(
            boost::asio::buffer(_server_msg, max_length),
            boost::bind(&KKClient::handle_read,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
}


void KKClient::close() {
    _socket.close();
}


void KKClient::handle_accept(const boost::system::error_code& error)
{
    listen();
}


void KKClient::handle_read(const boost::system::error_code& err, size_t bytes_transferred)
{
    if (!err) {
        cout << "[receive] - " << _server_msg << endl;
        listen();
    } else {
        std::cerr << "[error] - " << err.message() << std::endl;
        _socket.close();
    }
}

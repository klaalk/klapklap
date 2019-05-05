//
// Created by Klaus on 03/05/2019.
//

#ifndef SERVER_CON_HANDLER_H
#define SERVER_CON_HANDLER_H

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>



using namespace boost::asio;
using ip::tcp;
using std::cout;
using std::endl;

class con_handler: public boost::enable_shared_from_this<con_handler> {
private:
    tcp::socket sock;
    std::string message="Hello From Server!";
    enum { max_length = 1024 };
    char data[max_length];
public:
    typedef boost::shared_ptr<con_handler> pointer;
    // creating the pointer
    static pointer create(boost::asio::io_service& io_service)
    {
        return pointer(new con_handler(io_service));
    }
    /**
     * Socket creation
     */
    tcp::socket& socket();
    /**
     * Costruttore
     */
    con_handler(boost::asio::io_service& io_service);
    void start();
    void handle_read(const boost::system::error_code& err, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& err, size_t bytes_transferred);
};
#endif //SERVER_CON_HANDLER_H

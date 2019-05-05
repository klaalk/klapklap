//
// Created by Augens on 03/05/2019.
//

#include "con_handler.h"

con_handler::con_handler(boost::asio::io_service& io_service): sock(io_service){}

void con_handler::start()
{
    sock.async_read_some(
                boost::asio::buffer(data, max_length),
                boost::bind(&con_handler::handle_read,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));

    sock.async_write_some(
            boost::asio::buffer(data, max_length),
            boost::bind(&con_handler::handle_write,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));

}

void con_handler::handle_read(const boost::system::error_code& err, size_t bytes_transferred)
{
    if (!err) {
        cout << data << endl;
        start();
    } else {
        std::cerr << "error: " << err.message() << std::endl;
        sock.close();
    }
}

void con_handler::handle_write(const boost::system::error_code& err, size_t bytes_transferred)
{
    if (!err) {
        cout << "Server sent: I have recived " << data << endl;
    } else {
        std::cerr << "error: " << err.message() << endl;
        sock.close();
    }
}


//    Potrebbe essere utile per evitare attese
static void configureSocketTimeouts(boost::asio::ip::tcp::socket& socket)
{
#if defined(__APPLE__ )
    struct timeval tv;
    tv.tv_sec  = 15;
    tv.tv_usec = 0;
    setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#else
    int32_t timeout = 15000;
    setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
#endif
}

tcp::socket& con_handler::socket()
{
    return sock;
}

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <sstream>

#import <string.h>



using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::getline;

char msg2[1024];

void handle_read (const boost::system::error_code& err, size_t bytes_transferred)
{
    if (!err) {
        cout << "[debug] - " << msg2 << endl;

    } else {
        std::cerr << "error: " << err.message() << std::endl;

    }
}

int main() {
    boost::asio::io_service io_service;
    //socket creation
    tcp::socket socket(io_service);
    //connection
    socket.connect( tcp::endpoint( boost::asio::ip::address::from_string("192.168.0.101"), 3310 ));
    // request/message from client
    string msg = "";


    boost::system::error_code error;
    while (msg != "\n"){

        boost::asio::write( socket, boost::asio::buffer(msg, msg.size()), error );

        if( !error ) {
            cout << "Client sent: " << msg << endl;
        }
        else {
            cout << "send failed: " << error.message() << endl;
            break;
        }

        socket.async_read_some(
                boost::asio::buffer(msg2, 1024),
                boost::bind(&handle_read,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));

        if( !error ) {
            cout << "Server sent: " << msg2 << endl;
        }
        else {
            cout << "send failed: " << error.message() << endl;
            break;
        }
        getline(cin, msg);
    }
    socket.close();
    return 0;
}
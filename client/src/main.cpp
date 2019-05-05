#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <sstream>
#include <boost/enable_shared_from_this.hpp>
#import <string.h>

#import "../../libs/src/classes/connection_handler/con_handler.h"


using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::getline;

char data[1024];
int main() {
    enum { max_length = 1024 };
    boost::asio::io_service io_service;
    //socket creation
    tcp::socket socket(io_service);
    //connection
    socket.connect( tcp::endpoint( boost::asio::ip::address::from_string("192.168.0.101"), 3310 ));
    // request/message from client
    string msg = "Client Start :D";
    char msg2[max_length];

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
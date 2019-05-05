#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <sstream>
#include <boost/enable_shared_from_this.hpp>

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::getline;

char data[1024];
int main() {
    boost::asio::io_service io_service;
    //socket creation
    tcp::socket socket(io_service);
    //connection
    socket.connect( tcp::endpoint( boost::asio::ip::address::from_string("192.168.0.100"), 3310 ));
    // request/message from client
    string msg = "";

    boost::system::error_code error;
    do {
        getline(cin, msg);
        boost::asio::write( socket, boost::asio::buffer(msg), error );
        if( !error ) {
            cout << "Client sent: " << msg << endl;
        }
        else {
            cout << "send failed: " << error.message() << endl;
        }
    } while (msg.size() > 0);
    socket.close();
    return 0;
}
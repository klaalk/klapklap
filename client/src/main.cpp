#include <iostream>
#include <boost/asio.hpp>
#include <sstream>

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::getline;

int main() {
    boost::asio::io_service io_service;
    //socket creation
    tcp::socket socket(io_service);
    //connection
    socket.connect( tcp::endpoint( boost::asio::ip::address::from_string("192.168.1.11"), 3310 ));
    // request/message from client
    string msg = "";

    boost::system::error_code error;
    do {
        boost::asio::write( socket, boost::asio::buffer(msg), error );
        getline(cin, msg);
        if( !error ) {
            cout << "Client sent: " << msg << endl;
        }
        else {
            cout << "send failed: " << error.message() << endl;
        }

    } while (msg.size() > 0);



    //TODO: da gestire in un altro thread.
    // getting response from server
    boost::asio::streambuf receive_buffer;
    boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);
    if( error && error != boost::asio::error::eof ) {
        cout << "receive failed: " << error.message() << endl;
    }
    else {
        const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
        cout << data << endl;
    }


    return 0;
}
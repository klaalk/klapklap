//
//  main.cpp
//  Server
// prova
//  Created by Michele Luigi Greco on 01/05/2019.
//  Copyright © 2019 Michele Luigi Greco. All rights reserved.
//

#include <iostream>
#include <boost/chrono.hpp>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;

string read_(tcp::socket & socket) {
    boost::asio::streambuf buf;
    boost::asio::read_until( socket, buf, "\n" );                   //utile, si può impostare lettura fino ad un certo carattere, potremmo usare sequenza di escape come end dei vari messaggi.
    string data = boost::asio::buffer_cast<const char*>(buf.data());
    return data;
}
void send_(tcp::socket & socket, const string& message) {
    const string msg = message + "\n";
    boost::asio::write( socket, boost::asio::buffer(message) );
}

int main(int argc, const char * argv[]) {

    boost::asio::io_service io_service;

    //listen for new connection
    tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 3310 )); //Any IPv4, listen on port 1234

    //socket creation
    tcp::socket socket_(io_service);

    //waiting for connection
    acceptor_.accept(socket_);

    //read operation
    string message = read_(socket_);            //legge e stampa il contenuto della socket
    cout << message << endl;

    //write operation
    send_(socket_, "Hello From Server!");       //invia ciao da server
    cout << "Servent sent Hello message to Client!" << endl;
    return 0;
}

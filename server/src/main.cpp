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
#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;

//string read_(tcp::socket & socket) {
//    boost::asio::streambuf buf;
//    boost::asio::read_until( socket, buf, "\n" );                   //utile, si può impostare lettura fino ad un certo carattere, potremmo usare sequenza di escape come end dei vari messaggi.
//    string data = boost::asio::buffer_cast<const char*>(buf.data());
//    return data;
//}
//void send_(tcp::socket & socket, const string& message) {
//    const string msg = message + "\n";
//    boost::asio::write( socket, boost::asio::buffer(message) );
//}

int main(int argc, const char * argv[]) {

//    boost::asio::io_service io_service;
//
//    //listen for new connection
//    tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 3310 )); //Any IPv4, listen on port 1234
//
//    //socket creation
//    tcp::socket socket_(io_service);
//
//    //waiting for connection
//    acceptor_.accept(socket_);
//
//    //read operation
//    string message = read_(socket_);            //legge e stampa il contenuto della socket
//    cout << message << endl;
//
//    //write operation
//    send_(socket_, "Hello From Server!");       //invia ciao da server
//    cout << "Servent sent Hello message to Client!" << endl;

  sql::Driver *driver;
  sql::Connection *con;
  sql::Statement *stmt;
  sql::ResultSet *res;

  /* Create a connection */

  driver = get_driver_instance();
  con = driver->connect("tcp://130.192.163.109:3000", "server", "password");

  /* Connect to the MySQL test database */
  con->setSchema("KLAPKLAP_DB");

  stmt = con->createStatement();
  res = stmt->executeQuery("SELECT * FROM USERS");
  while (res->next()) {
//    cout << "\t... MySQL replies: ";
//    /* Access column data by alias or column name */
//    cout << res->getStatement() << endl;
    cout << "MySQL says: ";
    /* Access column data by numeric offset, 1 is the first column */
    cout << res->getString(1) + " " << res->getString(2) + " "<<res->getString(3) + " "<<res->getString(4) + " "<< res->getString(5) + " " << res->getString(6) + " "<<res->getString(7) + " "<<res->getString(8) + " "<<endl;
  }


    return 0;
}

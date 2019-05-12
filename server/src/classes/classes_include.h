//
// Created by jsnow on 12/05/19.
//

#ifndef SERVER_CLASSES_INCLUDE_H
#define SERVER_CLASSES_INCLUDE_H

//MySql Libraries
#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>

//HTTP Client Libraries
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

//Socket Libraries
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/chrono.hpp>
#include <boost/config.hpp>
#include <boost/asio.hpp>

//Generic Libraries
#include <string>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using std::string;
using std::cout;
using std::endl;

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

#endif //SERVER_CLASSES_INCLUDE_H

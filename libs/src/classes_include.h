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

//SMTP Service library
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/SMTPClientSession.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/InvalidCertificateHandler.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/MailRecipient.h>
using Poco::Net::InvalidCertificateHandler;
using Poco::Net::AcceptCertificateHandler;
using Poco::Net::Context;
using Poco::Net::SSLManager;
using Poco::Net::SecureStreamSocket;
using Poco::Net::SocketAddress;
using Poco::Net::SecureSMTPClientSession;
using Poco::Net::SMTPClientSession;
using Poco::SharedPtr;
using Poco::Net::MailMessage;
using Poco::Net::MailRecipient;

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
#include <cstring>


//Crypto
#include "../../../../.conan/data/OpenSSL/1.0.2o/conan/stable/package/7ee2b9bfcba105a6bcb3e1d60e3a0b24323ef7e1/include/openssl/evp.h"
#include "../../../../.conan/data/OpenSSL/1.0.2o/conan/stable/package/7ee2b9bfcba105a6bcb3e1d60e3a0b24323ef7e1/include/openssl/aes.h"
#include "../../../../.conan/data/OpenSSL/1.0.2o/conan/stable/package/7ee2b9bfcba105a6bcb3e1d60e3a0b24323ef7e1/include/openssl/err.h"


using std::string;
using std::cout;
using std::endl;

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>



#endif //SERVER_CLASSES_INCLUDE_H

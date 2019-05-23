//
// Created by jsnow on 12/05/19.
//

#ifndef SERVER_HTTP_SESSION_H
#define SERVER_HTTP_SESSION_H

#include "../../../../../libs/src/classes_include.h"

// Handles an HTTP server connection
class HTTP_session : public std::enable_shared_from_this<HTTP_session> {
    // This is the C++11 equivalent of a generic lambda.
    // The function object is used to send an HTTP message.
    struct send_lambda {
        HTTP_session &self_;

        explicit
        send_lambda(HTTP_session &self)
                : self_(self) {
        }

        template<bool isRequest, class Body, class Fields>
        void
        operator()(http::message<isRequest, Body, Fields> &&msg) const {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<
                    http::message<isRequest, Body, Fields >>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_.res_ = sp;

            // Write the response
            http::async_write(
                    self_.socket_,
                    *sp,
                    boost::asio::bind_executor(
                            self_.strand_,
                            std::bind(
                                    &HTTP_session::on_write,
                                    self_.shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2,
                                    sp->need_eof())));
        }
    };

    void fail(boost::system::error_code ec, char const *what);

    tcp::socket socket_;
    boost::asio::strand<
            boost::asio::io_context::executor_type> strand_;
    boost::beast::flat_buffer buffer_;
    std::string const &doc_root_;
    http::request<http::string_body> req_;
    std::shared_ptr<void> res_;
    send_lambda lambda_;

public:
    // Take ownership of the socket
    explicit
    HTTP_session(
            tcp::socket socket,
            std::string const &doc_root);

    // Start the asynchronous operation
    void run();

    void do_read();

    void on_read(
            boost::system::error_code ec,
            std::size_t bytes_transferred);

    void on_write(
            boost::system::error_code ec,
            std::size_t bytes_transferred,
            bool close);

    void do_close();
};

#endif //SERVER_HTTP_SESSION_H

//
// Created by Klaus on 06/05/2019.
//

#include "chat_session.h"

chat_session::chat_session(boost::asio::io_service& io_service, chat_room& room)
        : socket_(io_service),
          room_(room)
{
}

tcp::socket& chat_session::socket()
{
    return socket_;
}

void chat_session::start()
{
    room_.join(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.data(), message::header_length),
                            boost::bind(
                                    &chat_session::handle_read_header, shared_from_this(),
                                    boost::asio::placeholders::error));
}

void chat_session::deliver(const message& msg)
{
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 boost::bind(&chat_session::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error));
    }
}

void chat_session::handle_read_header(const boost::system::error_code& error)
{
    if (!error && read_msg_.decode_header())
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                boost::bind(&chat_session::handle_read_body, shared_from_this(),
                                            boost::asio::placeholders::error));

        std::cout << "handle_read_header" << std::endl;
        std::cout << read_msg_.data() << std::endl;
    }
    else
    {
        room_.leave(shared_from_this());
    }
}

void chat_session::handle_read_body(const boost::system::error_code& error)
{
    if (!error)
    {
        std::cout << "handle_read_body" << std::endl;
        std::cout << read_msg_.body() << std::endl;
        room_.deliver(read_msg_);
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), message::header_length),
                                boost::bind(&chat_session::handle_read_header, shared_from_this(),
                                            boost::asio::placeholders::error));
    }
    else
    {
        room_.leave(shared_from_this());
    }
}

void chat_session::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        std::cout << "handle_write" << std::endl;
        write_msgs_.pop_front();
        if (!write_msgs_.empty())
        {
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(write_msgs_.front().data(),
                                                         write_msgs_.front().length()),
                                     boost::bind(&chat_session::handle_write, shared_from_this(),
                                                 boost::asio::placeholders::error));
        }
    }
    else
    {
        room_.leave(shared_from_this());
    }
}

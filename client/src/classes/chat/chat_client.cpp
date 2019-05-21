//
// Created by Augens on 06/05/2019.
//

#include "chat_client.h"


chat_client::chat_client(boost::asio::io_service& io_service,
            tcp::resolver::iterator endpoint_iterator)
        : io_service_(io_service),
          socket_(io_service)
{
    tcp::endpoint endpoint = *endpoint_iterator;
    socket_.async_connect(endpoint,
                          boost::bind(&chat_client::handle_connect, this,
                                      boost::asio::placeholders::error, ++endpoint_iterator));
}

void chat_client::write(const message& msg)
{
    io_service_.post(boost::bind(&chat_client::do_write, this, msg));
}

void chat_client::close()
{
    io_service_.post(boost::bind(&chat_client::do_close, this));
}


void chat_client::handle_connect(const boost::system::error_code& error,
                    tcp::resolver::iterator endpoint_iterator)
{
    if (!error)
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), message::header_length),
                                boost::bind(&chat_client::handle_read_header, this,
                                            boost::asio::placeholders::error));
        // Effettuo il login
        char line[message::max_body_length + 1];
        std::cout<<"Inserisci username/email e password: <username/email> <password>"<<std::endl;
        std::cin.getline(line, 128);

        message msg;
        msg.body_length(strlen(line));
        memcpy(msg.body(), line, msg.body_length());
        msg.encode_header(login);
        write(msg);
    }
    else if (endpoint_iterator != tcp::resolver::iterator())
    {
        socket_.close();
        tcp::endpoint endpoint = *endpoint_iterator;
        socket_.async_connect(endpoint,
                              boost::bind(&chat_client::handle_connect, this,
                                          boost::asio::placeholders::error, ++endpoint_iterator));
    }
}

void chat_client::handle_read_header(const boost::system::error_code& _error)
{
    kk_payload_type _type = read_msg_.decode_header();
    if (!_error &&  _type != error )
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                boost::bind(&chat_client::handle_read_body, this,
                                            boost::asio::placeholders::error, _type));
    }
    else
    {
        do_close();
    }
}

void chat_client::handle_read_body(const boost::system::error_code& error, kk_payload_type _type)
{
    using namespace std; // For strlen and memcpy.
    char line[message::max_body_length + 1];
    if (!error)
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), message::header_length),
                                boost::bind(&chat_client::handle_read_header, this,
                                            boost::asio::placeholders::error));

        switch (_type) {
            case login: {
                if(strcmp(read_msg_.body(), "OK") == 0) {
                    char line[message::max_body_length + 1];
                    std::cout<<"Inserisci il nome del file che vuoi aprire:"<<std::endl;
                    std::cin.getline(line, 128);

                    message msg;
                    msg.body_length(strlen(line));
                    memcpy(msg.body(), line, msg.body_length());
                    msg.encode_header(openfile);
                    write(msg);
                } else {
                    std::cout<<"Il login non è andato a buon fine. Vuoi registrarti? y/n"<<std::endl;
                    char line[message::max_body_length + 1];
                    std::cout<<"WIP"<<std::endl;
                }
                break;
            }
            case openfile: {
                if(strcmp(read_msg_.body(), "OK") == 0) {
                    std::cout<<"Comandi possibili:"<<std::endl;
                    std::cout<<"<chat> per mandare una messaggio."<<std::endl;
                    std::cout<<"<crdt> per mandare un carattere."<<std::endl;
                    std::cout<<"<logout> per uscire."<<std::endl;
                    std::cin.getline(line, message::max_body_length + 1);

                    if(strcmp(line, "chat") == 0){
                        std::cout<<"Inserisci il corpo del messaggio:"<<std::endl;
                        std::cin.getline(line, message::max_body_length + 1);
                        message msg;
                        msg.body_length(strlen(line));
                        memcpy(msg.body(), line, msg.body_length());
                        msg.encode_header(chat);
                        write(msg);
                    } else if (strcmp(line, "crdt") == 0) {
                        std::cout<<"Inserisci <char> e <pos>:"<<std::endl;
                        std::cin.getline(line, message::max_body_length + 1);
                        message msg;
                        msg.body_length(strlen(line));
                        memcpy(msg.body(), line, msg.body_length());
                        msg.encode_header(crdt);
                        write(msg);
                    } else if (strcmp(line, "logout") == 0) {
                        char logoutmsg[] = "logout";
                        message msg;
                        msg.body_length(strlen(logoutmsg));
                        memcpy(msg.body(), logoutmsg, msg.body_length());
                        msg.encode_header(logout);
                        write(msg);
                    }
                }
            }
        }
    }
    else
    {
        do_close();
    }
}

void chat_client::do_write(message msg)
{
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 boost::bind(&chat_client::handle_write, this,
                                             boost::asio::placeholders::error));
    }
}

void chat_client::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        write_msgs_.pop_front();
        if (!write_msgs_.empty())
        {
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(write_msgs_.front().data(),
                                                         write_msgs_.front().length()),
                                     boost::bind(&chat_client::handle_write, this,
                                                 boost::asio::placeholders::error));
        }
    }
    else
    {
        do_close();
    }
}

void chat_client::do_close()
{
    socket_.close();
}

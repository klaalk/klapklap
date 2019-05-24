//
// Created by Augens on 06/05/2019.
//

#include "kk_client.h"


kk_client::kk_client(boost::asio::io_service& io_service,
            tcp::resolver::iterator endpoint_iterator)
        : io_service_(io_service),
          socket_(io_service)
{
    tcp::endpoint endpoint = *endpoint_iterator;
    socket_.async_connect(endpoint,
                          boost::bind(&kk_client::handle_connect, this,
                                      boost::asio::placeholders::error, ++endpoint_iterator));
}

void kk_client::write(const message& msg)
{
    io_service_.post(boost::bind(&kk_client::do_write, this, msg));
}

void kk_client::close()
{
    io_service_.post(boost::bind(&kk_client::do_close, this));
}

void kk_client::handle_connect(const boost::system::error_code& error,
                    tcp::resolver::iterator endpoint_iterator)
{
    if (!error)
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), message::header_length),
                                boost::bind(&kk_client::handle_read_header, this,
                                            boost::asio::placeholders::error));
        // Effettuo il login
        do_login();
    }
    else if (endpoint_iterator != tcp::resolver::iterator())
    {
        socket_.close();
        tcp::endpoint endpoint = *endpoint_iterator;
        socket_.async_connect(endpoint,
                              boost::bind(&kk_client::handle_connect, this,
                                          boost::asio::placeholders::error, ++endpoint_iterator));
    }
}

void kk_client::handle_read_header(const boost::system::error_code& _error)
{
    if (!_error &&  read_msg_.decode_header() != error )
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                boost::bind(&kk_client::handle_read_body, this,
                                            boost::asio::placeholders::error));
    }
    else
    {
        do_close();
    }
}

void kk_client::handle_read_body(const boost::system::error_code& error)
{
    using namespace std; // For strlen and memcpy.
    if (!error)
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), message::header_length),
                                boost::bind(&kk_client::handle_read_header, this,
                                            boost::asio::placeholders::error));
        receive_message();
    }
    else
    {
        do_close();
    }
}

void kk_client::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        write_msgs_.pop_front();
        if (!write_msgs_.empty())
        {
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(write_msgs_.front().data(),
                                                         write_msgs_.front().length()),
                                     boost::bind(&kk_client::handle_write, this,
                                                 boost::asio::placeholders::error));
        }
    }
    else
    {
        do_close();
    }
}

void kk_client::send_message(const char *line, kk_payload_type _type) {
    message msg;
    msg.body_length(strlen(line));
    memcpy(msg.body(), line, msg.body_length());
    msg.encode_header(_type, OK);
    std::cout <<"Sto inviando " << msg.data() << std::endl;
    write(msg);
}

void kk_client::receive_message(){
    char line[message::max_body_length + 1];

    std::cout <<"Ho ricevuto: " << read_msg_.data() << std::endl;
    switch (read_msg_.type()) {
        case login: {
            if( read_msg_.result_type() == OK) {
                std::cout<<"Inserisci il nome del file che vuoi aprire:"<<std::endl;
                std::cin.getline(line, message::max_body_length);
                send_message(line, openfile);
            } else {
                std::cout<<"Il login non è andato a buon fine. Vuoi registrarti? y/n"<<std::endl;
                std::cout<<"WIP"<<std::endl;
            }
            break;
        }
        case openfile: {
            if(read_msg_.result_type() == OK) {
                menu();
            } else {
                std::cout<<"Inserisci il nome del file che vuoi aprire:"<<std::endl;
                std::cin.getline(line, message::max_body_length);
                send_message(line, openfile);
            }
            break;
        }
        case crdt:
        case chat: {
            std::cout<< read_msg_.body()<<std::endl;
            menu();
            break;
        }
    }
    read_msg_.delete_data();
}

void kk_client::menu() {
    std::cout<<"Comandi possibili:"<<std::endl;
    std::cout<<"<openfile> per aprire una nuovo file;"<<std::endl;
    std::cout<<"<chat> per mandare una messaggio;"<<std::endl;
    std::cout<<"<crdt> per mandare un carattere;"<<std::endl;
    std::cout<<"<logout> per uscire;"<<std::endl;
    char line[message::max_body_length + 1];
    std::cin.getline(line, message::max_body_length);
    if(strcmp(line, "openfile") == 0){
        char _line[message::max_body_length + 1];
        std::cout<<"Inserisci il nome del file che vuoi aprire:"<<std::endl;
        std::cin.getline(_line, message::max_body_length);
        send_message(_line, openfile);
    } else if(strcmp(line, "chat") == 0){
        char _line[message::max_body_length + 1];
        std::cout<<"Inserisci il corpo del messaggio:"<<std::endl;
        std::cin.getline(_line, message::max_body_length);
        send_message(_line, chat);
    } else if (strcmp(line, "crdt") == 0) {
        char _line[message::max_body_length + 1];
        std::cout<<"Inserisci <char> e <pos>:"<<std::endl;
        std::cin.getline(_line, message::max_body_length);
        send_message(_line, crdt);
    } else if (strcmp(line, "logout") == 0) {
        char logoutmsg[] = "logout";
        send_message(logoutmsg, logout);
    }
}

void kk_client::do_login() {
    char line[message::max_body_length + 1];
    std::cout<<"Inserisci username/email e password: <username/email> <password>"<<std::endl;
    std::cin.getline(line, message::max_body_length);
    send_message(line, login);
}

void kk_client::do_write(message msg)
{
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 boost::bind(&kk_client::handle_write, this,
                                             boost::asio::placeholders::error));
    }
}

void kk_client::do_close()
{
    socket_.close();
}

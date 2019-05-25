//
// Created by Augens on 06/05/2019.
//

#include "kk_client.h"


kk_client::kk_client(boost::asio::io_service& io_service,
            tcp::resolver::iterator endpoint_iterator)
        : io_service_(io_service),
          socket_(io_service)
{
    connection_state = not_connected;
    state = file_closed;
    do_connection(endpoint_iterator);
}


kk_client_connection_state kk_client::get_connection_state() {
    return connection_state;
}

kk_client_state kk_client::get_state() {
    return state;
}

void kk_client::send_login() {
    char line[kk_payload::max_body_length + 1];
    std::cout<<"Inserisci username/email e password: <username/email> <password>"<<std::endl;
    std::cin.getline(line, kk_payload::max_body_length);
    sscanf(line, "%s %s", username, pass);
    send_kk_payload(line, login);
}

void kk_client::send_signup() {
    std::cout<<"Username non esistente. Vuoi registrarti? y/n"<<std::endl;
    std::cout<<"WIP"<<std::endl;
}

void kk_client::send_openfile() {
    char line[kk_payload::max_body_length + 1];
    std::cout<<"Inserisci il nome del file che vuoi aprire:"<<std::endl;
    std::cin.getline(line, kk_payload::max_body_length);
    send_kk_payload(line, openfile);
}

void kk_client::send_chat() {
    char _line[kk_payload::max_body_length + 1];
    std::cout<<"Inserisci il corpo del messaggio:"<<std::endl;
    std::cin.getline(_line, kk_payload::max_body_length);
    send_kk_payload(_line, chat);
}

void kk_client::send_crdt() {
    char _line[kk_payload::max_body_length + 1];
    std::cout<<"Inserisci <char> e <pos>:"<<std::endl;
    std::cin.getline(_line, kk_payload::max_body_length);
    send_kk_payload(_line, crdt);
}


void kk_client::close() {
    connection_state = connection_failed;
    cv.notify_one();
    io_service_.post(boost::bind(&kk_client::do_close, this));
}

void kk_client::send_kk_payload(const char *line, kk_payload_type _type) {
    kk_payload msg;
    msg.body_length(strlen(line));
    memcpy(msg.body(), line, msg.body_length());
    msg.encode_header(_type, OK);
//    std::cout <<"Sto inviando " << msg.data() << std::endl;
    connection_state = waiting;
    cv.notify_one();
    io_service_.post(boost::bind(&kk_client::do_write, this, msg));
}

void kk_client::receive_kk_payload() {
    char line[kk_payload::max_body_length + 1];
//    std::cout <<"Ho ricevuto: " << read_msg_.data() << std::endl;
    switch (read_msg_.type()) {
        case login: {
            if( read_msg_.result_type() == OK) {
                connection_state = authentication_succed;
                send_openfile();
            } else {
                connection_state = authentication_failed;
                send_signup();
            }
            break;
        }
        case openfile: {
            connection_state = authentication_succed;
            if(read_msg_.result_type() == OK) {
                state = file_opened;
            } else {
                send_openfile();
            }
            break;
        }
        case crdt: {
            connection_state = authentication_succed;
            print_chat();
            break;
        }
        case chat: {
            connection_state = authentication_succed;
            char _usr[128], _body[256];
            sscanf(read_msg_.body(), "%s: %s", _usr, _body);
            if(strcmp(_usr, username) != 0 ) {
                print_chat();
            }
            break;
        }
    }
    cv.notify_one();
    read_msg_.delete_data();
}

void kk_client::menu() {
    std::unique_lock<std::mutex> lk(cv_m);
    cv.wait(lk, [&]{
        if(connection_state == authentication_succed) {
            std::cout<<"Comandi possibili: <openfile> or <chat> or <crdt> or <logout>"<<std::endl;
            char line[kk_payload::max_body_length + 1];
            std::cin.getline(line, kk_payload::max_body_length);

            if(strcmp(line, "openfile") == 0){
                send_openfile();
            } else if(strcmp(line, "chat") == 0){
                send_chat();
            } else if (strcmp(line, "crdt") == 0) {
                send_crdt();
            } else if (strcmp(line, "logout") == 0) {
                char logoutmsg[] = "logout";
                send_kk_payload(logoutmsg, logout);
            }
            return true;
        } else if(connection_state == connection_failed) {
            return true;
        } else {
            return false;
        }
    });
}

// APERTURA CONNESSIONE
void kk_client::do_connection(tcp::resolver::iterator endpoint_iterator) {
    tcp::endpoint endpoint = *endpoint_iterator;
    socket_.async_connect(endpoint,
                          boost::bind(&kk_client::handle_connect, this,
                                      boost::asio::placeholders::error, ++endpoint_iterator));
}


void kk_client::handle_connect(const boost::system::error_code& error,
                               tcp::resolver::iterator endpoint_iterator) {
    if (!error) {
        do_read();
        connection_state = connected;
        send_login();
    }
    else if (endpoint_iterator != tcp::resolver::iterator()) {
        socket_.close();
        do_connection(endpoint_iterator);
    }
}

// LETTURA DAL SERVER
void kk_client::do_read() {
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.data(), kk_payload::header_length),
                            boost::bind(&kk_client::handle_read_header, this,
                                        boost::asio::placeholders::error));
}

void kk_client::handle_read_header(const boost::system::error_code& _error) {
    if (!_error &&  read_msg_.decode_header() != error ) {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                boost::bind(&kk_client::handle_read_body, this,
                                            boost::asio::placeholders::error));
    }
    else {
        do_close();
    }
}

void kk_client::handle_read_body(const boost::system::error_code& error) {
    using namespace std; // For strlen and memcpy.
    if (!error) {
        do_read();
        receive_kk_payload();
    }
    else {
        do_close();
    }
}

// SCRITTURA AL SERVER
void kk_client::do_write(kk_payload msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 boost::bind(&kk_client::handle_write, this,
                                             boost::asio::placeholders::error));
    }
}

void kk_client::handle_write(const boost::system::error_code& error) {
    if (!error) {
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

// CHIUSURA CONNESSIONE
void kk_client::do_close() {
    socket_.close();
}

void kk_client::print_chat() {
    if(read_msg_.type() == chat && read_msg_.result_type() == OK) {
        std::cout<< read_msg_.body()<<std::endl;
    }
}
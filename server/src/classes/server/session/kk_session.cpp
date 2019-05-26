//
// Created by Klaus on 06/05/2019.
//

#include "kk_session.h"
std::map<std::string, std::shared_ptr<kk_file>> files_;

kk_session::kk_session(boost::asio::io_service& io_service, kk_room& room)
        : socket_(io_service), room_(room)
{

}

tcp::socket& kk_session::socket()
{
    return socket_;
}

void kk_session::start()
{
    // mi aggiungo a tutti i partecipanti del server
    room_.join(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.data(), kk_payload::header_length),
                            boost::bind(
                                    &kk_session::handle_read_header, shared_from_this(),
                                    boost::asio::placeholders::error));
}

void kk_session::handle_read_header(const boost::system::error_code& _error)
{
    if (!_error && read_msg_.decode_header() != error)
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                boost::bind(&kk_session::handle_read_body, shared_from_this(),
                                            boost::asio::placeholders::error));
    }
    else
    {
        room_.leave(shared_from_this());
    }
}

void kk_session::handle_read_body(const boost::system::error_code& error)
{
    if (!error)
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), kk_payload::header_length),
                                boost::bind(&kk_session::handle_read_header, shared_from_this(),
                                            boost::asio::placeholders::error));
        handle_request();

    }
    else
    {
        actual_file_->leave(shared_from_this());
        room_.leave(shared_from_this());
    }
}

void kk_session::handle_request(){
    std::cout << "Ho ricevuto: " << read_msg_.data() << std::endl;
    switch (read_msg_.type()) {
        case login: {
            char usr[128];
            char psw[128];
            sscanf(read_msg_.body(), "%s %s", usr, psw);
            name = std::string(usr);
            //TODO: fare query e controllare se esiste.
            handle_response("Login effettuato", login, OK);
            break;
        }
        case openfile: {
            std::string filename = std::string(read_msg_.body());
            std::cout<<"richiesta di apertura file: " << filename << std::endl;
            //TODO: fare query e controllare se esiste.
            auto search = files_.find(filename);
            if(search != files_.end()) {
                // il file era già aperto ed è nella mappa globale
                actual_file_ = files_.at(filename);
                actual_file_->join(shared_from_this());
                handle_response("file esistente, sei stato aggiunto correttamente", openfile, OK);
            } else {
                // Apro il file. Con i dovuti controlli
                // TODO: fare query per inserire file
                actual_file_ = std::shared_ptr<kk_file>(new kk_file());
                actual_file_->join(shared_from_this());
                files_.insert(make_pair(filename, actual_file_));

                auto search = files_.find(filename);
                if(search != files_.end()) {
                    std::cout<< "file creato correttamente" << std::endl;
                    handle_response("file creato correttamente", openfile, OK);
                } else {
                    handle_response("non è stato possibile aprire il file", openfile, KO);
                }
            }

            break;
        }
        case chat: {
            std::string response = name + ": " + read_msg_.body();
            char cstr[response.size() + 1];

            response.copy(cstr, response.size() + 1);
            cstr[response.size()] = '\0';

            handle_response(cstr, chat, OK);
            break;
        }
    }

    read_msg_.delete_data();
}

void kk_session::handle_response(const char *body, kk_payload_type _type, kk_payload_result_type _result) {
    kk_payload msg;
    msg.body_length(strlen(body));
    memcpy(msg.body(), body, msg.body_length());
    msg.encode_header(_type, _result);
    if(_type == chat || _type == crdt) {
        // mando a tutti quelli registrati al file
        actual_file_->deliver(msg);

    } else {
        // mando al client di questa sessione
        deliver(msg);
    }
}

void kk_session::deliver(const kk_payload& msg)
{
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 boost::bind(&kk_session::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error));
    }
}

void kk_session::handle_write(const boost::system::error_code& error)
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
                                     boost::bind(&kk_session::handle_write, shared_from_this(),
                                                 boost::asio::placeholders::error));
        }
    }
    else
    {
        room_.leave(shared_from_this());
    }
}
//
// Created by Klaus on 06/05/2019.
//

#include "crdt_session.h"
std::map<std::string, crdt_file> files_;

crdt_session::crdt_session(boost::asio::io_service& io_service, crdt_room& room)
        : socket_(io_service), room_(room)
{

}

tcp::socket& crdt_session::socket()
{
    return socket_;
}

void crdt_session::start()
{
    // mi aggiungo a tutti i partecipanti del server
    room_.join(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.data(), message::header_length),
                            boost::bind(
                                    &crdt_session::handle_read_header, shared_from_this(),
                                    boost::asio::placeholders::error));
}

void crdt_session::handle_read_header(const boost::system::error_code& _error)
{
    kk_payload_type _type = read_msg_.decode_header();
    if (!_error && _type != error)
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                boost::bind(&crdt_session::handle_read_body, shared_from_this(),
                                            boost::asio::placeholders::error, _type));
    }
    else
    {
        room_.leave(shared_from_this());
    }
}

void crdt_session::handle_read_body(const boost::system::error_code& error, kk_payload_type _type)
{
    if (!error)
    {
        char _body[message::max_body_length];
        strncat(_body, read_msg_.body(), read_msg_.body_length());

        switch (_type) {
            case login: {
                char usr[128];
                char psw[128];
                sscanf(read_msg_.body(), "%s %s", usr, psw);
                std::cout<<"utente " << usr << " password " << psw << std::endl;
                //TODO: fare query e controllare se esiste.
                char logoutmsg[] = "OK";
                message msg;
                msg.body_length(strlen(logoutmsg));
                memcpy(msg.body(), logoutmsg, msg.body_length());
                msg.encode_header(login);
                deliver(msg);
                break;
            }
            case openfile: {
                char openfile_msg[] = "KO";
                std::string filename = std::string(_body);
                std::cout<<"richiesta di apertura file: " << filename << std::endl;
                //TODO: fare query e controllare se esiste.
                auto search = files_.find(filename);
                if(search != files_.end()) {
                    // il file era già aperto ed è nella mappa globale
                    actual_file_ = files_.at(filename);
                    actual_file_.join(shared_from_this());
                    strcpy(openfile_msg, "OK");
                } else {
                    // Apro il file. Con i dovuti controlli
                    // TODO: fare query per inserire file
                    actual_file_.join(shared_from_this());
                    files_.insert(make_pair(filename, actual_file_));

                    auto search = files_.find(filename);
                    if(search != files_.end()) {
                        std::cout<< "file creato correttamente" << std::endl;
                        strcpy(openfile_msg, "OK");
                    }
                }

                message msg;
                msg.body_length(strlen(openfile_msg));
                memcpy(msg.body(), openfile_msg, msg.body_length());
                msg.encode_header(openfile);
                deliver(msg);
                break;
            }
        }
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), message::header_length),
                                boost::bind(&crdt_session::handle_read_header, shared_from_this(),
                                            boost::asio::placeholders::error));
    }
    else
    {
        if(isInWriteMode_) {
            actual_file_.leave(shared_from_this());
        }
        room_.leave(shared_from_this());
    }
}

void crdt_session::deliver(const message& msg)
{
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 boost::bind(&crdt_session::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error));
    }
}

void crdt_session::handle_write(const boost::system::error_code& error)
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
                                     boost::bind(&crdt_session::handle_write, shared_from_this(),
                                                 boost::asio::placeholders::error));
        }
    }
    else
    {
        room_.leave(shared_from_this());
    }
}

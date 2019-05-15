//
// Created by Klaus on 06/05/2019.
//

#include "crdt_session.h"

std::map<std::string, crdt_file> files_;

crdt_session::crdt_session(boost::asio::io_service &io_service, crdt_room &room)
        : socket_(io_service), room_(room) {

}

tcp::socket &crdt_session::socket() {
    return socket_;
}

void crdt_session::start() {
    // mi aggiungo a tutti i partecipanti del server
    room_.join(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.data(), chat_message::header_length),
                            boost::bind(
                                    &crdt_session::handle_read_header, shared_from_this(),
                                    boost::asio::placeholders::error));
}

void crdt_session::handle_read_header(const boost::system::error_code &error) {
    if (!error && read_msg_.decode_header()) {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                boost::bind(&crdt_session::handle_read_body, shared_from_this(),
                                            boost::asio::placeholders::error));
    } else {
        room_.leave(shared_from_this());
    }
}

void crdt_session::handle_read_body(const boost::system::error_code &error) {
    if (!error) {
        if (!isInWriteMode_) {
            // Non ho nessun file aperto
            std::string fileName = std::string(read_msg_.body());
            auto search = files_.find(fileName);
            if (search != files_.end()) {
                // il file era già aperto ed è nella mappa globale
                actual_file_ = files_.at(fileName);
                actual_file_.join(shared_from_this());
            } else {
                // Apro il file. Con i dovuti controlli
                // TODO: actual_file.open(fileName);
                actual_file_.join(shared_from_this());
                files_.insert(make_pair(fileName, actual_file_));
                std::cout << "partecipante inserito" << std::endl;
            }
            isInWriteMode_ = true;
        } else {
            // Scrivo nel file.
            // TODO: actual_file_.writeOnFile(read_msg_.body());
            std::cout << "Scrivo nel file:\n" << read_msg_.body() << std::endl;
            actual_file_.deliver(read_msg_);
        }
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), chat_message::header_length),
                                boost::bind(&crdt_session::handle_read_header, shared_from_this(),
                                            boost::asio::placeholders::error));
    } else {
        if (isInWriteMode_) {
            actual_file_.leave(shared_from_this());
        }
        room_.leave(shared_from_this());
    }
}

void crdt_session::deliver(const chat_message &msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 boost::bind(&crdt_session::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error));
    }
}

void crdt_session::handle_write(const boost::system::error_code &error) {
    if (!error) {
        std::cout << "handle_write" << std::endl;
        write_msgs_.pop_front();
        if (!write_msgs_.empty()) {
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(write_msgs_.front().data(),
                                                         write_msgs_.front().length()),
                                     boost::bind(&crdt_session::handle_write, shared_from_this(),
                                                 boost::asio::placeholders::error));
        }
    } else {
        actual_file_.leave(shared_from_this());
        room_.leave(shared_from_this());
    }
}

//
// Created by Klaus on 06/05/2019.
//

#include "kk_session.h"

#include "QtWebSockets/QWebSocketServer"
#include "QtWebSockets/QWebSocket"
#include <QtCore/QDebug>
#include <QtCore/QFile>


#define DEBUG


std::map<std::string, std::shared_ptr<kk_file>> files_;

kk_session::kk_session(std::shared_ptr<kk_db> db, QWebSocket*  pSocket)
        : pSocket(pSocket), db_(db) {

}

void kk_session::handle_request() {

/*
#ifdef DEBUG
    std::cout << "Ho ricevuto: " << read_msg_.data() << std::endl;
#endif
    switch (read_msg_.type()) {
        case login: {
            char usr[128];
            char psw[128];
            sscanf(read_msg_.body(), "%s %s", usr, psw);
#ifdef DEBUG
            std::cout << usr << " "<< psw << std::endl;
#endif
            name = std::string(usr);
            db_->db_login(QString::fromStdString(name),QString::fromStdString(psw)) ?
            handle_response("Login effettuato", login, OK) :
            handle_response("Password errata!", login, KO);
            break;
        }
        case openfile: {
            std::string filename = std::string(read_msg_.body());
            std::cout << "richiesta di apertura file: " << filename << std::endl;
            //TODO: fare query e controllare se esiste.
            auto search = files_.find(filename);
            if (search != files_.end()) {
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
                if (search != files_.end()) {
                    std::cout << "file creato correttamente" << std::endl;
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
*/
}

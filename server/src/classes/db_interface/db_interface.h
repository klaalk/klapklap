//
// Created by jsnow on 12/05/19.
//

#ifndef SERVER_DB_INTERFACE_H
#define SERVER_DB_INTERFACE_H

#include "../classes_include.h"
#include "../db_connector/db_connector.h"
#include "./HTTP_listener/HTTP_listener.h"

#define HOST_IP "tcp://130.192.163.109"
#define HOST_DOC_ROOT "."
#define HOST_N_THREAD 5
#define HOST_DOOR  3000


class db_interface {
private:
    // Utilizzato per il singleton
    // Da utilizzare per fare le query
    std::unique_ptr<db_connector> connector;
    // Da utilizzare per gestire le richieste http
    std::shared_ptr<HTTP_listener> listener;
    boost::asio::io_context ioc_;
    std::unique_ptr<db_interface> interface;

public:
    db_interface();

    void start();
};


#endif //SERVER_DB_INTERFACE_H

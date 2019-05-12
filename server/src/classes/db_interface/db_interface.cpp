//
// Created by jsnow on 12/05/19.
//

#include "db_interface.h"

db_interface::db_interface() {
    connector = std::unique_ptr<db_connector>(new db_connector(get_driver_instance()));

    // Create and launch a listening port
    listener = std::make_shared<HTTP_listener>(
            ioc_,
            tcp::endpoint(),
            HOST_DOC_ROOT);

    ioc_.run();
}

void db_interface::start() {
    listener->run();
}
//
// Created by jsnow on 12/05/19.
//

#include "db_interface.h"

db_interface::db_interface() {

}

void db_interface::start() {
    listener->run();
}
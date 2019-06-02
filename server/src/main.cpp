//
//  main.cpp
//  Server
//
//  Created by Michele Luigi Greco on 01/05/2019.
//  Copyright © 2019 Michele Luigi Greco. All rights reserved.
//

#include <QApplication>
#include <QDebug>

#include "classes/server/kk_server.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    kk_server server(3310);
    Q_UNUSED(server);
    return a.exec();
}



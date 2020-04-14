//
//  main.cpp
//  Server
//

#include <QApplication>
#include <QDebug>
#include "classes/server/kk_server.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    KKServer server(3002);
    Q_UNUSED(server)
    return a.exec();
}

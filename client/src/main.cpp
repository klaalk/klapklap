#include <QApplication>
#include <QPalette>
#include "classes/client/kk_client.h"

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(client);
    QApplication a(argc, argv);
   KKClient client(QUrl(QStringLiteral("wss://localhost:3002")));
//   KKClient client(QUrl(QStringLiteral("wss://130.192.163.109:3002")));
    a.setWindowIcon(QIcon(":/klapklap.icns"));
    Q_UNUSED(client);
    return a.exec();
}



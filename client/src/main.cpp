#include <QApplication>
#include <QPalette>
#include <QtCore/QDebug>
#include <QFont>
#include <algorithm>

#include "../../libs/src/classes/crdt/kk_crdt.h"
#include "../../libs/src/classes/crdt/char/kk_char.h"

#include "classes/client/kk_client.h"

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(client);
    QApplication app(argc, argv);

    // Apply the loaded stylesheet
    QFile styleFile( ":/styles/styles.qss");
    styleFile.open( QFile::ReadOnly );
    app.setStyleSheet(QString( styleFile.readAll()));

    // Local Server
    KKClient client(QUrl(QStringLiteral("wss://localhost:3002")));

    // Public Server
    //KKClient client(QUrl(QStringLiteral("wss://93.56.104.204:3002")));

    QApplication::setWindowIcon(QIcon(":/klapklap.icns"));

    Q_UNUSED(client)
    return QApplication::exec();
}


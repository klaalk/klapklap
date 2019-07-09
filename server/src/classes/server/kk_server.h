//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_SERVER_H
#define KK_SERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtNetwork/QSslError>

#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>

#include "session/kk_session.h"
#include "classes/server/filesys/kk_filesys.h"
#include "file/kk_file.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)


class KKServer : public QObject {
    Q_OBJECT
public:
    KKServer(quint16 port, QObject *parent = nullptr);
    ~KKServer() override;
private slots:
    void onNewConnection();
    void onSslErrors(const QList<QSslError> &errors);
private:
    QWebSocketServer* socket;
    QList<QWebSocket*> clients;
    QList<KKSessionPtr> sessions;
    KKMapFilePtr files;
    kk_db_ptr db;
    KKFileSystemPtr filesys;
};

#endif //KK_SERVER_H

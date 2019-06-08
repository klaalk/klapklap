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
#include "file/kk_file.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)


class kk_server : public QObject {
Q_OBJECT
public:
    kk_server(quint16 port, QObject *parent = nullptr);
    ~kk_server() override;
private slots:
    void onNewConnection();
    void onSslErrors(const QList<QSslError> &errors);
private:
    QWebSocketServer* server_socket_;
    QList<QWebSocket*> clients_;
    QList<kk_session_ptr> sessions_;
    map_files_ptr files_;
    kk_db_ptr db_;
};

#endif //KK_SERVER_H

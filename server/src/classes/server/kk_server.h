//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_SERVER_H
#define KK_SERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtNetwork/QSslError>

#include "session/kk_session.h"
#include "file/kk_file.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)


class kk_server : public QObject {
Q_OBJECT
public:
    kk_server(quint16 port, QObject *parent = nullptr);
    ~kk_server() override;
private Q_SLOTS:
    void onNewConnection();

    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);

    void socketDisconnected();
    void onSslErrors(const QList<QSslError> &errors);
private:
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;

    std::shared_ptr<kk_db> db_;
};

typedef std::shared_ptr<kk_server> kk_server_ptr;
#endif //KK_SERVER_H

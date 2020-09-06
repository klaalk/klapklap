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
#include "../../libs/src/constants/kk_constants.h"

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
    QString generateSessionId();
    QWebSocketServer* socket;
    QList<QWebSocket*> clients;
    QList<KKSessionPtr> sessions;
    KKMapFilePtr files;
    KKDataBasePtr db;
    KKFileSystemPtr filesys;
    KKFilePtr logFile;

    QString possibleCharacters;
    int randomStringLength = 12; // assuming you want random strings of 12 characters

};

#endif //KK_SERVER_H

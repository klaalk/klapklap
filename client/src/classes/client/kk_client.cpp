//
// Created by Augens on 06/05/2019.
//

#include "kk_client.h"

#include "../../../../libs/src/classes/crypt/kk_crypt.h"
#include <QtCore/QDebug>
#include <QtWebSockets/QWebSocket>
#include <QCoreApplication>

kk_client::kk_client(const QUrl &url, QObject *parent)
    : QObject(parent)
{
    connect(&socket_, &QWebSocket::connected, this, &kk_client::onConnected);
    connect(&socket_, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
            this, &kk_client::onSslErrors);
    socket_.open(QUrl(url));
}

void kk_client::sendLoginRequest(QString email, QString password) {
    SimpleCrypt solver(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString psw = solver.encryptToString(password);
    kk_payload req("login", "ok", email + "_" + password);
    qDebug() << "Send: " << req.encode_header();
    socket_.sendTextMessage(req.encode_header());
}

void kk_client::sendOpenFileRequest(QString fileName) {
    kk_payload req("openfile", "ok", fileName);
    socket_.sendTextMessage(req.encode_header());
}

void kk_client::onConnected()
{
    qDebug() << "WebSocket connected";
    // Gestisco la lettura dei messaggi.
    connect(&socket_, &QWebSocket::textMessageReceived, this, &kk_client::onMessageReceived);
    connect(&view_, &MainWindow::loginBtnClicked, this, &kk_client::sendLoginRequest);
    view_.show();
}

void kk_client::onMessageReceived(QString message)
{
    qDebug() << "Message received:" << message;
    kk_payload res(message);
    res.decode_header();
    if(res.type() == "login" && res.result_type() == "ok") {
       view_.hide();
       //TODO: aprire la window che gestisce i file.
       // ma per il momento faccio l'apertura automatica del file.

       sendOpenFileRequest("file1");

//       view_.openEditor();
    }
}

void kk_client::onSslErrors(const QList<QSslError> &errors)
{
    Q_UNUSED(errors);
    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.
    socket_.ignoreSslErrors();
}

void kk_client::closeConnection(){
    qApp->quit();
}

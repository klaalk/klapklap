//
// Created by Klaus on 06/05/2019.
//

#ifndef CLIENT_CHAT_MESSAGE_H
#define CLIENT_CHAT_MESSAGE_H
#include <deque>
#include <QString>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>
#include <QtCore/QDebug>

#include "../../constants/kk_constants.h"

class KKPayload {
public:
    enum {
        max_body_length = 512
    };
    /// Costruttuore.
    KKPayload();
    KKPayload(QString data);
    KKPayload(QString request, QString result, const QStringList& bodyList);

    QString getData();
    QString getBody();
    QStringList getBodyList();
    QString getHeader();
    QString getRequestType();
    QString getResultType();
    int getTotalLength();

    QString decode();
    QString encode();

private:
    QString data;
    QString header;
    QString body;
    QString request;
    QString result;
    size_t bodyLength{};
};

typedef QSharedPointer<KKPayload> KKPayloadPtr;
typedef QSharedPointer<QVector<KKPayload>> KKVectorPayloadPtr;
typedef QSharedPointer<QVector<QPair<KKPayload, QString>>> KKVectorPairPayloadPtr;
#endif //CLIENT_CHAT_MESSAGE_H

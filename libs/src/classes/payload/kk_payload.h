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
#include "../../constants/kk_constants.h"

class KKPayload {
public:
    enum {
        max_body_length = 512
    };

    /// Costruttuore.
    KKPayload(QString data);
    KKPayload(QString type, QString result, QString message);

    QString getBody();

    int getTotalLength();

    QString getType();

    QString getResultType();

    QString decodeHeader();

    QString encodeHeader();

private:
    QString data;
    QString msg;
    QString type;
    QString result;
    size_t bodyLength;
};

typedef QSharedPointer<KKPayload> KKPayloadPtr;
typedef QSharedPointer<QVector<KKPayloadPtr>> KKVectorPayloadPtr;
#endif //CLIENT_CHAT_MESSAGE_H

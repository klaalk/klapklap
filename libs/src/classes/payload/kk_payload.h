//
// Created by Klaus on 06/05/2019.
//

#ifndef CLIENT_CHAT_MESSAGE_H
#define CLIENT_CHAT_MESSAGE_H

#include <QString>
#include <QStringList>
#include "../../constants/kk_constants.h"

class kk_payload {
public:
    enum {
        max_body_length = 512
    };

    /// Costruttuore.
    kk_payload(QString data);
    kk_payload(QString type, QString result, QString message);

    QString body();

    QString type();

    QString result_type();

    QString decode_header();

    QString encode_header();

private:
    QString data_;
    QString msg_;
    QString type_;
    QString result_;

    size_t body_length_;
};

#endif //CLIENT_CHAT_MESSAGE_H

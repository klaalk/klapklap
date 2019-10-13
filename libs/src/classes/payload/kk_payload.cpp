//
// Created by Klaus on 06/05/2019.
//

#include "kk_payload.h"

KKPayload::KKPayload(QString data)
    : data(data) {
}


KKPayload::KKPayload(QString type, QString result, QString message)
    : msg(message), type(type), result(result), bodyLength(message.size()) {
}

QString KKPayload::getBody(){
    return msg;
}

int KKPayload::getTotalLength() {
    return data.toLatin1().length();
}

QString KKPayload::getType() {
    return type;
}

QString KKPayload::getResultType() {
    return result;
}

QString KKPayload::decodeHeader() {
    QStringList list = data.split("-");

    type = list.at(0);
    result = list.at(1);
    msg = list.at(2);

    return type;
}

QString KKPayload::encodeHeader() {
    return data= type + "-" + result + "-" + msg;
}

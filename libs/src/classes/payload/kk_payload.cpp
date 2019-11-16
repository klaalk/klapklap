//
// Created by Klaus on 06/05/2019.
//

#include "kk_payload.h"

KKPayload::KKPayload(QString data)
    : data(data) {
}


KKPayload::KKPayload(QString request, QString result, QStringList bodyList)
    : request(request), result(result) {

    for (QString value : bodyList){
        body.append(QString(PAYLOAD_FORMAT).arg(value.length(), PAYLOAD_FORMAT_LENGTH, 10, QChar('0')) +
                    value);
    }

    bodyLength = static_cast<size_t>(body.length());
}

QString KKPayload::getData() {
    return data;
}

QString KKPayload::getBody() {
    return body;
}

QStringList KKPayload::getBodyList() {
    QStringList list;
    int start = 0;
    int nextFieldLenth = 0;
    do {
        nextFieldLenth = body.mid(start, PAYLOAD_FORMAT_LENGTH).toInt();
        start += PAYLOAD_FORMAT_LENGTH;
        QString field = body.mid(start, nextFieldLenth);
        if (field.length() > 0) list.append(field);
        start += nextFieldLenth;
    }
    while (nextFieldLenth > 0);
    return list;
}


QString KKPayload::getHeader() {
    return header;
}

int KKPayload::getTotalLength() {
    return data.toLatin1().length();
}

QString KKPayload::getRequestType() {
    return request;
}

QString KKPayload::getResultType() {
    return result;
}

QString KKPayload::decode() {
    header = data.mid(0, PAYLOAD_HEAD_LENGTH);
    request = header.mid(0, PAYLOAD_REQUEST_LENGTH);
    result = header.mid(PAYLOAD_REQUEST_LENGTH, PAYLOAD_RESULT_LENGTH);

    body = data.mid(PAYLOAD_HEAD_LENGTH, data.length() - PAYLOAD_HEAD_LENGTH + 1);
    return request;
}

QString KKPayload::encode() {
    return data = request + result + " " + body + PAYLOAD_END;
}

//
// Created by Klaus on 06/05/2019.
//

#include "kk_payload.h"
#include <utility>

KKPayload::KKPayload(){}

KKPayload::KKPayload(QString data)
    : data(std::move(data)) {
}


KKPayload::KKPayload(QString request, QString result, const QStringList& bodyList)
    : request(std::move(request)), result(std::move(result)) {

    for (const QString& value : bodyList) {
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
    int nextFieldLength = 0;
    do {
        nextFieldLength = body.midRef(start, PAYLOAD_FORMAT_LENGTH).toInt();
        start += PAYLOAD_FORMAT_LENGTH;
        QString field = body.mid(start, nextFieldLength);
        if (field.length() > 0) list.append(field);
        start += nextFieldLength;
    }
    while (start < body.size());
    return list;
}


QString KKPayload::getHeader() {
    return header;
}

int KKPayload::getTotalLength() {
    return data.length();
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

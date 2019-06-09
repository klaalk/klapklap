//
// Created by Klaus on 06/05/2019.
//

#include "kk_payload.h"

kk_payload::kk_payload(QString data)
        : data_(data) {
}


kk_payload::kk_payload(QString type, QString result, QString message)
        : msg_(message), type_(type), result_(result), body_length_(message.size()) {
}

QString kk_payload::body(){
    return msg_;
}

QString kk_payload::type() {
    return type_;
}

QString kk_payload::result_type() {
    return result_;
}

QString kk_payload::decode_header() {
    QStringList list = data_.split("-");

    type_ = list.at(0);
    result_ = list.at(1);
    msg_ = list.at(2);

    return type_;
}

QString kk_payload::encode_header() {
    return data_= type_ + "-" + result_ + "-" + msg_;
}

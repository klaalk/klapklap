//
// Created by Klaus on 06/05/2019.
//

#include "message.h"

#define MESSAGE_CODE "%d-%3d-%4d"

message::message()
        : body_length_(0) {
}

const char *message::data() const {
    return data_;
}

char *message::data() {
    return data_;
}

size_t message::length() const {
    return header_length + body_length_;
}

const char *message::body() const {
    return data_ + header_length;
}

char *message::body() {
    return data_ + header_length;
}

size_t message::body_length() const {
    return body_length_;
}

void message::body_length(size_t length) {
    body_length_ = length;
    if (body_length_ > max_body_length)
        body_length_ = max_body_length;
}

void message::delete_data() {
    memset(data_, '\0', header_length + body_length_);
    body_length_ = 0;
}

kk_payload_type message::type() {
    return type_;
}

kk_payload_result_type message::result_type(){
    return  result_;
}

kk_payload_type message::decode_header() {
    using namespace std; // For strncat and atoi.
    char header[header_length + 1] = "";
    kk_payload_type _type;
    kk_payload_result_type _result;
    int _length = 0;

    strncat(header, data_, header_length);
    sscanf(header, MESSAGE_CODE, &_type, &_result, &_length);

    type_ = _type;
    if (_length > max_body_length) {
        body_length_ = 0;
        result_ = ERR_SIZE;
        return error;
    }
    body_length_ = _length;
    result_ = _result;

    return type_;
}

void message::encode_header(kk_payload_type type, kk_payload_result_type result) {
    using namespace std; // For sprintf and memcpy.
    char header[header_length + 1] = "";
    sprintf(header, MESSAGE_CODE,(int)type, (int)result, (int)body_length_);
    memcpy(data_, header, header_length);
}

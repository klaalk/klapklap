//
// Created by Klaus on 06/05/2019.
//

#include "message.h"

#define MESSAGE_CODE "%d%4d"

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
    body_length_ = 0;
    data_[0] = 0;
}

kk_payload_type message::decode_header() {
    using namespace std; // For strncat and atoi.
    char header[header_length + 1] = "";
    kk_payload_type type_;
    int length_;

    strncat(header, data_, header_length);
    sscanf(header, MESSAGE_CODE, &type_, &length_);
    if (length_ > max_body_length) {
        body_length_ = 0;
        return error;
    }
    body_length_ = length_;
    return type_;
}

void message::encode_header(kk_payload_type type) {
    using namespace std; // For sprintf and memcpy.
    char header[header_length + 1] = "";
    sprintf(header, MESSAGE_CODE,(int)type, (int)body_length_);
    memcpy(data_, header, header_length);
}

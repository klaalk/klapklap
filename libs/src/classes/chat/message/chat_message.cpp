//
// Created by Klaus on 06/05/2019.
//

#include "chat_message.h"


chat_message::chat_message()
        : body_length_(0) {
}

const char *chat_message::data() const {
    return data_;
}

char *chat_message::data() {
    return data_;
}

size_t chat_message::length() const {
    return header_length + body_length_;
}

const char *chat_message::body() const {
    return data_ + header_length;
}

char *chat_message::body() {
    return data_ + header_length;
}

size_t chat_message::body_length() const {
    return body_length_;
}

void chat_message::body_length(size_t length) {
    body_length_ = length;
    if (body_length_ > max_body_length)
        body_length_ = max_body_length;
}

bool chat_message::decode_header() {
    using namespace std; // For strncat and atoi.
    char header[header_length + 1] = "";
    strncat(header, data_, header_length);
    body_length_ = atoi(header);
    if (body_length_ > max_body_length) {
        body_length_ = 0;
        return false;
    }
    return true;
}

void chat_message::encode_header() {
    using namespace std; // For sprintf and memcpy.
    char header[header_length + 1] = "";
    sprintf(header, "%4d", body_length_);
    memcpy(data_, header, header_length);
}

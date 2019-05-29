//
// Created by Klaus on 06/05/2019.
//

#include "kk_payload.h"

#define MESSAGE_CODE "%d-%3d-%4d"

kk_payload::kk_payload()
        : body_length_(0) {
}

const char *kk_payload::data() const {
    return data_;
}

char *kk_payload::data() {
    return data_;
}

size_t kk_payload::length() const {
    return header_length + body_length_;
}

const char *kk_payload::body() const {
    return data_ + header_length;
}

char *kk_payload::body() {
    return data_ + header_length;
}

size_t kk_payload::body_length() const {
    return body_length_;
}

void kk_payload::body_length(size_t length) {
    body_length_ = length;
    if (body_length_ > max_body_length)
        body_length_ = max_body_length;
}

void kk_payload::delete_data() {
    memset(data_, '\0', header_length + body_length_);
    body_length_ = 0;
}

kk_payload_type kk_payload::type() {
    return type_;
}

kk_payload_result_type kk_payload::result_type() {
    return result_;
}

kk_payload_type kk_payload::decode_header() {
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

void kk_payload::encode_header(kk_payload_type type, kk_payload_result_type result) {
    using namespace std; // For sprintf and memcpy.
    char header[header_length + 1] = "0";
    sprintf(header, MESSAGE_CODE, (int) type, (int) result, (int) body_length_);
    memcpy(data_, header, header_length);
}

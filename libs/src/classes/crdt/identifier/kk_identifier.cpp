//
// Created by Alberto Bruno on 2019-05-15.
//
#include "kk_identifier.h"

using std::string;

kk_identifier::kk_identifier(int digit, string siteid) : digit(digit), siteid(siteid) {};

int kk_identifier::compare_to(const kk_identifier &other) {
    if (this->digit < other.digit) {
        return -1;
    } else if (this->digit > other.digit) {
        return 1;
    } else if (this->siteid < other.siteid) {
        return -1;
    }
    return 1;
}

int kk_identifier::get_digit() {
    return this->digit;
}

void kk_identifier::set_digit(int digit) {
    this->digit = digit;
    return;
}

string kk_identifier::get_siteid() {
    return this->siteid;
}

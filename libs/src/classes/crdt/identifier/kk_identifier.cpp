//
// Created by Alberto Bruno on 2019-05-15.
//
#include "kk_identifier.h"
#include <utility>
using std::string;

KKIdentifier::KKIdentifier(unsigned long digit, string siteid) : digit(digit), siteid(std::move(siteid)) {};

int KKIdentifier::compareTo(const KKIdentifier &other) {
    if (this->digit < other.digit) {
        return -1;
    } if (this->digit > other.digit) {
        return 1;
    } else if (this->siteid < other.siteid) {
        return -1;
    } else if (this->siteid > other.siteid) {
        return 1;
    }
    return 0;
}

unsigned long KKIdentifier::getDigit() {
    return this->digit;
}

void KKIdentifier::setDigit(unsigned long digit) {
    this->digit = digit;
}

string KKIdentifier::getSiteId() {
    return this->siteid;
}

//
// Created by Alberto Bruno on 2019-05-15.
//
#include "../identifier/kk_identifier.h"
#include "kk_char.h"

using std::string;

kk_char::kk_char(char value, string siteId) : siteId(siteId), value(value) {};

void kk_char::push_identifier(kk_identifier_ptr id) {
    this->position.insert(this->position.end(), id);
    return;
}

/*void kk_char::insert_identifier(std::vector<identifier>::iterator it,identifier x) {
    this->position.insert(it,x);
    return;
}*/

int kk_char::compare_to(const kk_char &other) {
    int min, comp;

    if (this->position.size() < other.position.size()) {
        min = static_cast<int>(this->position.size());
    } else {
        min = static_cast<int>(other.position.size());
    }

    for (int i = 0; i < min; i++) {
        comp = this->position[static_cast<unsigned long>(i)]->compare_to(*other.position[static_cast<unsigned long>(i)].get());
        if (comp != 0) {
            return comp;
        }
    }

    if (this->position.size() < other.position.size()) {
        return -1;
    } else if (this->position.size() > other.position.size()) {
        return 1;
    } else {
        return 0;
    }
}

char kk_char::get_value() {
    return value;
}
//string kk_char::get_siteId() {
//    return siteId;
//}

void kk_char::insert_position(vector<kk_identifier_ptr> position) {
    this->position = position;
    return;
}

vector<kk_identifier_ptr> kk_char::get_position() {
    return this->position;
}

std::string kk_char::get_identifiers_string() {
    std::string identifiers_ = "";
    for(int i = 0; i < this->position.size(); i++) {
        identifiers_+= "_" + this->position.at(i)->get_digit();
    }
    return identifiers_;
}

//
// Created by Alberto Bruno on 2019-05-15.
//
#include "../identifier/kk_identifier.h"
#include "kk_char.h"

using std::string;

kk_char::kk_char(char value, string siteId) : siteId(siteId), value(value) {};

void kk_char::push_identifier(kk_identifier_ptr id) {
    position.push_back(id);
}

/*void kk_char::insert_identifier(std::vector<identifier>::iterator it,identifier x) {
    this->position.insert(it,x);
    return;
}*/

int kk_char::compare_to(const kk_char &other) {
    int comp;
    unsigned long min;

    if (this->position.size() <= other.position.size()) {
        min = this->position.size();
    } else {
        min = other.position.size();
    }

    for (unsigned long i = 0; i < min; i++) {
        kk_identifier id1(*this->position[i].get());
        kk_identifier id2(*other.position[i].get());

         comp=id1.compare_to(id2);

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
string kk_char::get_siteId() {
    return siteId;
}

void kk_char::insert_position(vector<kk_identifier_ptr> position) {
    this->position = position;
    return;
}

vector<kk_identifier_ptr> kk_char::get_position() {
    return this->position;
}

std::string kk_char::get_identifiers_string() {
    std::string identifiers_ = "";
    std::for_each(position.begin(), position.end(),[&](kk_identifier_ptr i){
        char str[sizeof(unsigned long)+1];
        sprintf(str, "%lu", i->get_digit());
        identifiers_= identifiers_ + "_" + str;
    });
    return identifiers_;
}

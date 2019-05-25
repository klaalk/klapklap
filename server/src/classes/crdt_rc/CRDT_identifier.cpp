//
// Created by Alberto Bruno on 2019-05-15.
//
#include "CRDT_identifier.h"

using std::string;

CRDT_identifier::CRDT_identifier(int digit, string siteid):digit(digit),siteid(siteid){};

int CRDT_identifier::compare_to(const CRDT_identifier &other)
{
    if(this->digit<other.digit){
        return -1;
    }
    else if(this->digit > other.digit){
        return 1;
    }
    else if(this->siteid < other.siteid){
        return -1;
    }
    return 1;
}

int CRDT_identifier::get_digit(){
    return this->digit;
}

void CRDT_identifier::set_digit(int digit){
    this->digit=digit;
    return;
}

string CRDT_identifier::get_siteid() {
    return this->siteid;
}

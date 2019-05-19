//
// Created by Alberto Bruno on 2019-05-15.
//
#include "CRDT_Identifier.h"
#include <stdio.h>
#include <stdlib.h>
using std::vector;
using std::string;

identifier::identifier(int digit, string siteid):digit(digit),siteid(siteid){};

int identifier::compareTo(const identifier &other)
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

int identifier::get_digit(){
    return this->digit;
}

void identifier::set_digit(int digit){
    this->digit=digit;
    return;
}

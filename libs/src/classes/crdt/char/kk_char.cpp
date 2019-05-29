//
// Created by Alberto Bruno on 2019-05-15.
//
#include "../identifier/kk_identifier.h"
#include "kk_char.h"

using std::string;

kk_char::kk_char(char value, string siteId):value(value), siteId(siteId) {};

void kk_char::push_identifier(kk_identifier id) {
    this->position.insert(this->position.end(),id);
    return;
}

/*void kk_char::insert_identifier(std::vector<identifier>::iterator it,identifier x) {
    this->position.insert(it,x);
    return;
}*/

int kk_char::compare_to(const kk_char& other){
    int min,comp;

    if(this->position.size()<other.position.size()){
        min=this->position.size();
    }else{
        min=other.position.size();
    }

    for(int i=0;i<min;i++){

        comp = this->position[i].compare_to(other.position[i]);

        if (comp!=0){
            return comp;
        }
    }
    if(this->position.size()<other.position.size()){
        return -1;
    } else if(this->position.size()>other.position.size()){
        return 1;
    } else {
        return 0;
    }
}

char kk_char::get_value() {
    return value;
}

void kk_char::insert_position(vector<kk_identifier> position) {
    this->position = position;
    return;
}

vector<kk_identifier> kk_char::get_position(){
    return this->position;
}
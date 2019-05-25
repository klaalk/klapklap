//
// Created by Alberto Bruno on 2019-05-15.
//
#include "CRDT_identifier.h"
#include "CRDT_Char.h"

using std::string;

CRDT_Char::CRDT_Char(char value, string siteId):value(value), siteId(siteId) {};

void CRDT_Char::push_identifier(CRDT_identifier id) {
    this->position.insert(this->position.end(),id);
    return;
}

/*void CRDT_Char::insert_identifier(std::vector<identifier>::iterator it,identifier x) {
    this->position.insert(it,x);
    return;
}*/

int CRDT_Char::compare_to(const CRDT_Char& other){
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

char CRDT_Char::get_value() {
    return value;
}

void CRDT_Char::insert_position(vector<CRDT_identifier> position) {
    this->position = position;
    return;
}

vector<CRDT_identifier> CRDT_Char::get_position(){
    return this->position;
}
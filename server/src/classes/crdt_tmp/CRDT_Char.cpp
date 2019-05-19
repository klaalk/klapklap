//
// Created by Alberto Bruno on 2019-05-15.
//
#include "CRDT_Identifier.h"
#include <stdio.h>
#include <stdlib.h>
#include "CRDT_Char.h"
#include <vector>

using std::vector;
using std::string;

CRDT_Char::CRDT_Char(char value, string siteId):value(value), siteId(siteId) {};

void CRDT_Char::push_identifier(identifier id) {
    this->position.insert(this->position.end(),id);
    return;
}

/*void CRDT_Char::insert_identifier(std::vector<identifier>::iterator it,identifier x) {
    this->position.insert(it,x);
    return;
}*/

int CRDT_Char::compareTo(const CRDT_Char& other){
    int min,comp;

    if(this->position.size()<other.position.size()){
        min=this->position.size();
    }else{
        min=other.position.size();
    }

    for(int i=0;i<min;i++){

        comp = this->position[i].compareTo(other.position[i]);

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

void CRDT_Char::insert_position(vector<identifier> position) {
    this->position = position;
    return;
}

vector<identifier> CRDT_Char::get_position(){
    return this->position;
}
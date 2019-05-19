//
// Created by Alberto Bruno on 2019-05-12.
//
#include "CRDT_Identifier.h"
#include <stdio.h>
#include <stdlib.h>
#include "Char.h"
#define DIMPOSITION 10

Char::Char(char value, std::string siteId,int counter): value(value), siteId(siteId), counter(counter){};

int Char::compareTo(const Char& other){
    int min,comp;
    identifier id1,id2;

    if(position.size()<other.position.size()){ //per trovare la char con meno cifre (serve per il ciclo for di dopo)
        min=position.size();
    }else{
        min=other.position.size();
    }

    for(int i=0;i<min;i++){

        id1=position[i];
        id2=other.position[i];
        comp = id1.compareTo(id2);

        if (comp!=0){
            return comp;
        }
    }
    if(position.size()<other.position()){
        return -1;
    } else if(position.size()>other.position()){
        return 1;
    } else {
        return 0;
    }
}


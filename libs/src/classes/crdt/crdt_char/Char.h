//
// Created by Alberto Bruno on 2019-05-12.
//
#include "CRDT_Identifier.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#define DIMPOSITION 10

#ifndef SERVER_CHAR_H
#define SERVER_CHAR_H


class Char {
    std::vector<identifier> position(DIMPOSITION); // array di identifier, serve per dare come posizioni univoche numeri anche con la virgola (es 0,5 -> [identifier1.digit=0][identifier2.digit=5])
    std::string siteId; //univoco per il client
    char value; //valore (es 'a')
    int counter;

public:
    Char(char value, std::string siteId,int counter);
    int compareTo(const Char& other);
};


#endif //SERVER_CHAR_H

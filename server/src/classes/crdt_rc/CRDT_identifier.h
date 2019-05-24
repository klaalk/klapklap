//
// Created by Alberto Bruno on 2019-05-15.
//
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <iostream>


#ifndef SERVER_IDENTIFIER_H
#define SERVER_IDENTIFIER_H

using std::vector;
using std::string;


class CRDT_identifier{
    int digit; // cifra della posizione
    string siteid; //univoco del client, serve per calcolare posizione in caso di stessi digit

public:
    CRDT_identifier(int digit, string siteid); //costruttore
    int compare_to(const CRDT_identifier& other); // torna -1 se questo < l'altro, 1 se questo > l'altro e lo fa confrontando prima il digit e poi se il digit è uguale, il siteId
    int get_digit();
    void set_digit(int digit);
    string get_siteid();
};


#endif //SERVER_IDENTIFIER_H

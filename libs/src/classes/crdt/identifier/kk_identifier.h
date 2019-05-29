//
// Created by Alberto Bruno on 2019-05-15.
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>


#ifndef KK_IDENTIFIER_H
#define KK_IDENTIFIER_H

using std::string;

class kk_identifier {
    int digit; // cifra della posizione
    string siteid; //univoco del client, serve per calcolare posizione in caso di stessi digit

public:
    kk_identifier(int digit, string siteid); //costruttore
    int compare_to(
            const kk_identifier &other); // torna -1 se questo < l'altro, 1 se questo > l'altro e lo fa confrontando prima il digit e poi se il digit è uguale, il siteId
    int get_digit();

    void set_digit(int digit);

    string get_siteid();
};


#endif //KK_IDENTIFIER_H

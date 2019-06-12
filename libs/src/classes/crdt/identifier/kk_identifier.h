//
// Created by Alberto Bruno on 2019-05-15.
//
#ifndef KK_IDENTIFIER_H
#define KK_IDENTIFIER_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <memory>

using std::string;
class kk_identifier {
    unsigned long digit; // cifra della posizione
    string siteid; //univoco del client, serve per calcolare posizione in caso di stessi digit
public:
    kk_identifier(unsigned long digit, string siteid); //costruttore
    int compare_to(const kk_identifier &other); // torna -1 se questo < l'altro, 1 se questo > l'altro e lo fa confrontando prima il digit e poi se il digit è uguale, il siteId
    unsigned long get_digit();
    void set_digit(unsigned long digit);
    string get_siteid();
};

typedef std::shared_ptr<kk_identifier> kk_identifier_ptr;
#endif //KK_IDENTIFIER_H

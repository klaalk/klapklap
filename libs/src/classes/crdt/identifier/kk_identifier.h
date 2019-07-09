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
class KKIdentifier {
    unsigned long digit; // cifra della posizione
    string siteid; //univoco del client, serve per calcolare posizione in caso di stessi digit
public:
    KKIdentifier(unsigned long digit, string siteid); //costruttore
    int compareTo(const KKIdentifier &other); // torna -1 se questo < l'altro, 1 se questo > l'altro e lo fa confrontando prima il digit e poi se il digit è uguale, il siteId
    unsigned long getDigit();
    void setDigit(unsigned long digit);
    string getSiteId();
};

typedef std::shared_ptr<KKIdentifier> KKIdentifierPtr;
#endif //KK_IDENTIFIER_H

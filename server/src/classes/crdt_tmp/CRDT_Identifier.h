//
// Created by Alberto Bruno on 2019-05-15.
//

#include <stdio.h>
#include <stdlib.h>
#include <string>

#ifndef SERVER_IDENTIFIER_H
#define SERVER_IDENTIFIER_H

using std::vector;
using std::string;


class identifier{
    int digit; // cifra della posizione
    string siteid; //univoco del client, serve per calcolare posizione in caso di stessi digit

public:
    identifier(int digit, string siteid); //costruttore
    int compareTo(const identifier& other); // torna -1 se questo < l'altro, 1 se questo > l'altro e lo fa confrontando prima il digit e poi se il digit è uguale, il siteId
    int get_digit();
    void set_digit(int digit);
};


#endif //SERVER_IDENTIFIER_H

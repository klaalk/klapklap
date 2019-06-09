//
// Created by Alberto Bruno on 2019-05-15.
//
#ifndef KK_CHAR_H
#define KK_CHAR_H

#include "../identifier/kk_identifier.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <iostream>
#include <memory>

using std::vector;
using std::string;

class kk_char {
    vector<kk_identifier_ptr> position; // array di identifier, serve per dare come posizioni univoche numeri anche con la virgola (es 0,5 -> [identifier1.digit=0][identifier2.digit=5])
    string siteId; //univoco per il client
    char value; //valore (es 'a')
public:
    kk_char(char value, string siteId);//costruttore

    char get_value();
    int compare_to(const kk_char &other); //torna: 1 - questa > l'altra, -1 l'altra > questa, 0 se no
    void push_identifier(kk_identifier_ptr x); //mette l'identifier nuovo come ultimo elemento del vettore posizione(aggiunge una cifra)
    void insert_identifier(vector<kk_identifier_ptr>::iterator it, kk_identifier_ptr id);
    void insert_position(vector<kk_identifier_ptr> position);
    vector<kk_identifier_ptr> get_position();
};

typedef std::shared_ptr<kk_char> kk_char_ptr;

#endif //KK_CHAR_H

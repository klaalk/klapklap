//
// Created by Alberto Bruno on 2019-05-15.
//
#include "CRDT_Identifier.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
using std::vector;
using std::string;


#ifndef SERVER_CHAR_H
#define SERVER_CHAR_H


class CRDT_Char {
    vector<identifier> position; // array di identifier, serve per dare come posizioni univoche numeri anche con la virgola (es 0,5 -> [identifier1.digit=0][identifier2.digit=5])
    string siteId; //univoco per il client
    char value; //valore (es 'a')


public:

    CRDT_Char(char value, string siteId);//costruttore
    int compareTo(const CRDT_Char& other); //torna: 1 - questa > l'altra, -1 l'altra > questa, 0 se no
    void push_identifier(identifier x); //mette l'identifier nuovo come ultimo elemento del vettore posizione(aggiunge una cifra)
    void insert_identifier(vector<identifier>::iterator it,identifier id);
    void insert_position(vector<identifier> position);
    vector<identifier> get_position();

};

#endif

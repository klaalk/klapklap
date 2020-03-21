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
#include <algorithm>
#include <string.h>
#include <memory>
#include <QTextCharFormat>

using std::vector;
using std::string;

class KKChar {
    vector<KKIdentifierPtr> position; // array di identifier, serve per dare come posizioni univoche numeri anche con la virgola (es 0,5 -> [identifier1.digit=0][identifier2.digit=5])
    string siteId; //univoco per il client
    char value; //valore (es 'a')
    //QTextCharFormat KKCharFormat;
    QString KKCharFont;

public:
    KKChar(char value, string siteId);//costruttore
    ~KKChar();
    char getValue();
    string getSiteId();
    void insertSiteId(string siteId);
    int compareTo(const KKChar &other); //torna: 1 - questa > l'altra, -1 l'altra > questa, 0 se no
    void pushIdentifier(KKIdentifierPtr x); //mette l'identifier nuovo come ultimo elemento del vettore posizione(aggiunge una cifra)
    void insertIdentifier(vector<KKIdentifierPtr>::iterator it, KKIdentifierPtr id);
    void insertPosition(vector<KKIdentifierPtr> position);
    vector<KKIdentifierPtr> getPosition();
    std::string getIdentifiersString();
    QString getKKCharFont();
    void setKKCharFont(QString font);
     /*QTextCharFormat getKKCharFormat();
     void setKKCharFormat(QTextCharFormat);*/
};

typedef std::shared_ptr<KKChar> KKCharPtr;

#endif //KK_CHAR_H

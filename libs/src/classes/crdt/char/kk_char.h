//
// Created by Alberto Bruno on 2019-05-15.
//
#ifndef KK_CHAR_H
#define KK_CHAR_H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <memory>
#include <sstream>
#include <utility>

#include <QTextCharFormat>

#include "../identifier/kk_identifier.h"

using std::vector;

using std::stringstream;

class KKChar {
    vector<KKIdentifierPtr> position; // array di identifier, serve per dare come posizioni univoche numeri anche con la virgola (es 0,5 -> [identifier1.digit=0][identifier2.digit=5])
    QString siteId; //univoco per il client
    QChar value; //valore (es 'a')
    QString KKCharFont;
    QString KKCharColor;

public:
    KKChar(QChar value, QString siteId);
    KKChar(QChar value, QString siteId, QString KKCharFont, QString KKCharColor);
    ~KKChar();
    QChar getValue();
    QString getSiteId();
    void insertSiteId(QString siteId);
    int compareTo(const KKChar &other); //torna: 1 - questa > l'altra, -1 l'altra > questa, 0 se no
    void pushIdentifier(const KKIdentifierPtr& x); //mette l'identifier nuovo come ultimo elemento del vettore posizione(aggiunge una cifra)
    void insertIdentifier(vector<KKIdentifierPtr>::iterator it, KKIdentifierPtr id);
    void insertPosition(vector<KKIdentifierPtr> position);
    vector<KKIdentifierPtr> getPosition();
    QString encodeIdentifiers();
    void decodeIdentifiers(QString encodedIds);
    QString getKKCharFont();
    void setKKCharFont(QString font);
    QString getKKCharColor();
    void setKKCharColor(QString color);

};

typedef std::shared_ptr<KKChar> KKCharPtr;

#endif //KK_CHAR_H

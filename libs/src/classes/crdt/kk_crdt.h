//
// Created by Alberto Bruno on 2019-05-15.
//
#ifndef KK_CRDT_H
#define KK_CRDT_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <list>
#include <thread>
#include <sys/time.h>
#include <cstdlib>
#include <ctime>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

#include <QCharRef>
#include <QDebug>
#include <QString>

#include "char/kk_char.h"
#include "pos/kk_pos.h"
#include "identifier/kk_identifier.h"

using std::list;
using std::vector;
using std::string;

enum KKStrategy {
    casuale, minus, plus
};

class KKCrdt {
public:
    KKCrdt(string siteid, KKStrategy strategy);
    ~KKCrdt();

    QStringList encodeCrdt();
    void decodeCrdt(QStringList crdt);

    KKCharPtr localInsert(char val, KKPosition pos, QString font, QString color);
    list<KKCharPtr> localDelete(KKPosition startPos, KKPosition endPos);

    unsigned long remoteInsert(const KKCharPtr& charPtr);
    unsigned long remoteDelete(const KKCharPtr& charPtr);
    unsigned long remoteFormatChange(const KKCharPtr& _char,QString font_, QString color_);

    list<KKCharPtr> changeMultipleKKCharFormat(KKPosition start, KKPosition end,QString font_, QString color_);//cambia il formato di tutte le KKChar comprese tra le due posizioni e restituisce la lista delle KKChar cambiate
    KKCharPtr changeSingleKKCharFormat(KKPosition pos, QString font_, QString color_);

    void calculateLineCol(unsigned long global_pos, unsigned long *line, unsigned long *col);
    unsigned long calculateGlobalPosition(KKPosition pos);

    void setLineAlignment(unsigned long idx, int align);
    int getLineAlignment(unsigned long idx);
    bool checkLine(unsigned long lineIdx);

    vector<int> getLinesAlignment() const;
    vector<list<KKCharPtr>> getText() const;
    QString getSiteId() const;
    bool isTextEmpty();

    void print();
    void printText();
    void printForm();
    void printLinesAlignment();

private:
    /// Inserimento Char nel crdt
    void insertChar(const KKCharPtr& charPtr, KKPosition pos);
    /// Genera la Char partendo dal valore e dalla posizione nel local text
    KKCharPtr generateChar(char val, KKPosition pos);
    /// Partendo dalle position di due Char(adiacenti) genera la posizione della Char
    vector<KKIdentifierPtr> generatePositionBetween(vector<KKIdentifierPtr> position1, vector<KKIdentifierPtr> position2, vector<KKIdentifierPtr> *new_position, unsigned long level);
    /// Dati due identifier ne genera uno nuovo da mettere nella position della nuova Char usando la strategia opportuna
    unsigned long generateIdentifierBetween(unsigned long min, unsigned long max, KKStrategy _strategy, unsigned long level);

    /// Trova poszione del char
    KKPosition findPosition (const KKCharPtr& charPtr, bool *flag);
    /// Trava posizione di inserimento per il char
    KKPosition findInsertPosition (const KKCharPtr& charPtr);
    /// Trova posizione ultimo char
    KKPosition findEndPosition (KKChar lastChar);
    // Trova la position della Char immediatamente prima di quella passata
    vector<KKIdentifierPtr> findPositionBefore(KKPosition pos);
    // Trova la position della Char immediatamente dopo di quella passata
    vector<KKIdentifierPtr> findPositionAfter(KKPosition pos);
    /// Trova l'indice del char nella lina
    unsigned long findIndexInLine(const KKCharPtr& charPtr, list<KKCharPtr> line, bool *flag);
    /// Trova indice per inserire il char nella linea
    unsigned long findInsertIndexInLine(const KKCharPtr& charPtr, list<KKCharPtr> line);
    // Trova la strategia migliore per assegnare un identifier alla position della nuova Char
    KKStrategy findStrategy();

    /// Unisco le line
    void mergeLines(unsigned long line);

    /// Cancello piu' linee
    list<KKCharPtr> deleteMultipleLines(KKPosition start_pos, KKPosition end_pos);
    /// Cacnello linee vuote
    list<KKCharPtr> deleteSingleLine(KKPosition start_pos, KKPosition end_pos);
    /// Cancello le linee vuote
    void deleteEmptyLines();

    /// Separo le line
    vector<KKIdentifierPtr> slice(vector<KKIdentifierPtr> const &v,int i);
    /// Creo un nuovo Char da quello passato per parametro
    KKCharPtr copyChar(KKChar* source);

    string siteId;
    KKStrategy strategy;
    vector<KKStrategy> strategyCache;
    vector<list<KKCharPtr>> text;
    vector<int> linesAlignment;
    unsigned long boundary;
    unsigned long base;
};

typedef QSharedPointer<KKCrdt> KKCrdtPtr;

#endif //KK_CRDT_H

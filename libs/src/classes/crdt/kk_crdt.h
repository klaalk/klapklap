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

#include "char/kk_char.h"
#include "pos/kk_pos.h"
#include "identifier/kk_identifier.h"

#include "../../kklibexport.h"

using std::list;
using std::vector;
using std::string;

enum strategy {
    casuale, minus, plus
};

class KKCrdt {
public:
    vector<list<KKCharPtr>> text;
    string siteid;
    unsigned long boundary;
    strategy _strategy;
    vector<strategy> strategy_cache;
    unsigned long base;

    KKCrdt(string siteid, strategy strategy); //costruttore

    void insertChar(KKCharPtr _char, KKPosition pos);
    KKCharPtr localInsert(char val, KKPosition pos);
    KKCharPtr generateChar(char val, KKPosition pos); //genera la Char partendo dal valore e dalla posizione nel local text
    vector<KKIdentifierPtr> generatePositionBetween(vector<KKIdentifierPtr> position_1, vector<KKIdentifierPtr> position_2, vector<KKIdentifierPtr> *new_position, unsigned long livello); //partendo dalle position di due Char(adiacenti) genera la posizione della Char
    unsigned long generateIdentifierBetween(unsigned long min, unsigned long max, strategy _strategy, unsigned long level); //dati due identifier ne genera uno nuovo da mettere nella position della nuova Char usando la strategia opportuna
    unsigned long remoteInsert(KKCharPtr _char);

    vector<KKIdentifierPtr> findPositionBefore(KKPosition pos); //trova la position della Char immediatamente prima di quella passata
    vector<KKIdentifierPtr> findPositionAfter(KKPosition pos);//trova la position della Char immediatamente dopo di quella passata

    void mergeLines(unsigned long line);

    int isEmpty();
    void removeEmptyLines();

    list<KKCharPtr> localDelete(KKPosition start_pos, KKPosition end_pos);
    list<KKCharPtr> deleteMultipleLines(KKPosition start_pos, KKPosition end_pos);
    list<KKCharPtr> deleteSingleLine(KKPosition start_pos, KKPosition end_pos);
    unsigned long remoteDelete(KKCharPtr _Char);

    KKPosition findInsertPosition(KKCharPtr _char);
    KKPosition findPos (KKCharPtr _Char,bool *flag);
    KKPosition findEndPosition (KKChar last_char,  list<KKCharPtr> last_line, unsigned long total_lines);
    unsigned long findIndexInLine(KKCharPtr _Char, list<KKCharPtr> line,bool *flag);
    unsigned long findInsertIndexInLine(KKCharPtr _char, list<KKCharPtr> line);

    unsigned long generateGlobalPos(KKPosition pos);
    void calculateLineCol(unsigned long global_pos, unsigned long *line, unsigned long *col);

    vector<KKIdentifierPtr> slice(vector<KKIdentifierPtr> const &v,int i);//FORSE DA TOGLIERE
    void print();
    strategy findStrategy(unsigned long level);//trova la strategia migliore per assegnare un identifier alla position della nuova Char
};


#endif //KK_CRDT_H

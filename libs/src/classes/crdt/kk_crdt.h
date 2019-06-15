//
// Created by Alberto Bruno on 2019-05-15.
//
#ifndef KK_CRDT_H
#define KK_CRDT_H


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <list>
#include <thread>

#include <QRandomGenerator>

#include "char/kk_char.h"
#include "pos/kk_pos.h"
#include "identifier/kk_identifier.h"

using std::list;
using std::vector;
using std::string;

enum strategy {
    casuale, minus, plus
};

class kk_crdt {
public:
    vector<list<kk_char_ptr>> text;
    string siteid;
    unsigned long boundary;
    strategy _strategy;
    vector<strategy> strategy_cache;
    unsigned long base;
    //costruttore
    kk_crdt(string siteid, strategy strategy);
    void print();

    kk_char_ptr local_insert(char val, kk_pos pos);
    void local_delete(kk_pos start_pos, kk_pos end_pos);

    kk_pos remote_insert(kk_char_ptr _char);
    void remote_delete(kk_char_ptr _Char, string siteid);
private:
    QRandomGenerator gen;
    ///genera la Char partendo dal valore e dalla posizione nel local text
    kk_char_ptr generate_Char(char val, kk_pos pos);
    ///trova la position della Char immediatamente prima di quella passata
    vector<kk_identifier_ptr> find_position_before(kk_pos pos);
    ///trova la position della Char immediatamente dopo di quella passata
    vector<kk_identifier_ptr> find_position_after(kk_pos pos);
    //partendo dalle position di due Char(adiacenti) genera la posizione della Char
    vector<kk_identifier_ptr> generate_position_between(vector<kk_identifier_ptr> position_1, vector<kk_identifier_ptr> position_2,
                                                    vector<kk_identifier_ptr> *new_position, unsigned long livello);

    //trova la strategia migliore per assegnare un identifier alla position della nuova Char
    strategy find_strategy(unsigned long level);
    //dati due identifier ne genera uno nuovo da mettere nella position della nuova Char usando la strategia opportuna
    unsigned long generate_identifier_between(unsigned long min, unsigned long max, strategy _strategy);
    void insert_char(kk_char_ptr _char, kk_pos pos);
    kk_pos find_insert_position(kk_char_ptr _char);
//    int is_empty();
    kk_pos find_end_position (kk_char last_char,  list<kk_char_ptr> last_line, unsigned long total_lines);
    unsigned long find_insert_index_in_line(kk_char_ptr _char, list<kk_char_ptr> line);
    list<kk_char_ptr> delete_multiple_lines(kk_pos start_pos, kk_pos end_pos);
    list<kk_char_ptr> delete_single_line(kk_pos start_pos, kk_pos end_pos);
    void merge_lines(unsigned long line);
    void remove_empty_lines();
    kk_pos find_pos (kk_char_ptr _Char,bool flag);
    int find_index_in_line(kk_char_ptr _Char, list<kk_char_ptr> line,bool flag);
    vector<kk_identifier_ptr> slice(vector<kk_identifier_ptr> const &v,int i);//FORSE DA TOGLIERE
};

typedef std::shared_ptr<kk_crdt> kk_crdt_ptr;
#endif //KK_CRDT_H

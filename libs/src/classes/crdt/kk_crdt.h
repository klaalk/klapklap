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

class kk_crdt {
public:
    vector<list<kk_char_ptr>> text;
    string siteid;
    unsigned long boundary;
    strategy _strategy;
    vector<strategy> strategy_cache;
    unsigned long base;

    kk_crdt(string siteid, strategy strategy); //costruttore

    kk_char_ptr local_insert(char val, kk_pos pos);

    kk_char_ptr generate_Char(char val, kk_pos pos); //genera la Char partendo dal valore e dalla posizione nel local text

    vector<kk_identifier_ptr> find_position_before(kk_pos pos); //trova la position della Char immediatamente prima di quella passata

    vector<kk_identifier_ptr> find_position_after(kk_pos pos);//trova la position della Char immediatamente dopo di quella passata
    vector<kk_identifier_ptr> generate_position_between(vector<kk_identifier_ptr> position_1, vector<kk_identifier_ptr> position_2,
                                                    vector<kk_identifier_ptr> *new_position, unsigned long livello); //partendo dalle position di due Char(adiacenti) genera la posizione della Char
    strategy find_strategy(unsigned long level);//trova la strategia migliore per assegnare un identifier alla position della nuova Char
    unsigned long generate_identifier_between(unsigned long min, unsigned long max, strategy _strategy, unsigned long level); //dati due identifier ne genera uno nuovo da mettere nella position della nuova Char usando la strategia opportuna
    void insert_char(kk_char_ptr _char, kk_pos pos);
    kk_pos remote_insert(kk_char_ptr _char);
    kk_pos find_insert_position(kk_char_ptr _char);
    int is_empty();
    kk_pos find_end_position (kk_char last_char,  list<kk_char_ptr> last_line, unsigned long total_lines);
    unsigned long find_insert_index_in_line(kk_char_ptr _char, list<kk_char_ptr> line);
    list<kk_char_ptr> local_delete(kk_pos start_pos, kk_pos end_pos);
    list<kk_char_ptr> delete_multiple_lines(kk_pos start_pos, kk_pos end_pos);
    list<kk_char_ptr> delete_single_line(kk_pos start_pos, kk_pos end_pos);
    void merge_lines(unsigned long line);
    void remove_empty_lines();
    kk_pos remote_delete(kk_char_ptr _Char);
    kk_pos find_pos (kk_char_ptr _Char,bool *flag);
    unsigned long find_index_in_line(kk_char_ptr _Char, list<kk_char_ptr> line,bool *flag);
    vector<kk_identifier_ptr> slice(vector<kk_identifier_ptr> const &v,int i);//FORSE DA TOGLIERE
    void print();
    unsigned long generate_global_pos(kk_pos pos);
};


#endif //KK_CRDT_H

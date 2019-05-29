//
// Created by Alberto Bruno on 2019-05-15.
//

#include "identifier/kk_identifier.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <list>

#include "char/kk_char.h"
#include "pos/kk_pos.h"

#ifndef KK_CRDT_H
#define KK_CRDT_H

using std::list;
using std::vector;
using std::string;

enum strategy {casuale, minus, plus};

class kk_crdt {
    vector<list<kk_char>> text;
    string siteid;
    int boundary;
    strategy _strategy;
    vector<strategy> strategy_cache;
    int base;
    vector<kk_identifier> slice(vector<kk_identifier> const &v, int i);
public:
    kk_crdt(string siteid,strategy strategy); //costruttore
    void local_insert(char val,kk_pos pos );
    kk_char generate_Char(char val, kk_pos pos);
    vector<kk_identifier> find_position_before(kk_pos pos);
    vector<kk_identifier> find_position_after(kk_pos pos);
    vector<kk_identifier> generate_position_between(vector<kk_identifier> position_1,vector<kk_identifier> position_2,vector<kk_identifier> *new_position,int livello);
    strategy find_strategy(int level);
    int generate_identifier_between(int min, int max,strategy _strategy);
//    void handle_remote_insert(kk_char _char);
//    kk_pos find_insert_position(kk_char _char);
//    int is_empty();
//    kk_pos find_end_position (kk_char last_char,  list<kk_char> last_line, int total_lines);
//    int find_insert_index_in_line(kk_char _char, list<kk_char> line);
    void insert_char(kk_char _char, kk_pos pos);

    void print();
};


#endif //KK_CRDT_H

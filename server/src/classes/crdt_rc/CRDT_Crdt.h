//
// Created by Alberto Bruno on 2019-05-15.
//

#include "CRDT_identifier.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "CRDT_Char.h"
#include <vector>
#include "CRDT_pos.h"

#ifndef UNTITLED_CRDT_H
#define UNTITLED_CRDT_H

using std::vector;
using std::string;

enum strategy {casuale, minus, plus};

class CRDT_Crdt {

vector<vector<CRDT_Char>> text;
string siteid;
int boundary;
strategy _strategy;
vector<strategy> strategy_cache;
int base;


public:
    CRDT_Crdt(string siteid,int boundary,strategy strategy,int base); //costruttore
    void local_insert(char val,CRDT_pos pos );
    CRDT_Char generate_Char(char val, CRDT_pos pos);
    vector<CRDT_identifier> find_position_before(CRDT_pos pos);
    vector<CRDT_identifier> find_position_after(CRDT_pos pos);
    vector<CRDT_identifier> generate_position_between(vector<CRDT_identifier> position_1,vector<CRDT_identifier> position_2,vector<CRDT_identifier> *new_position,int livello);
    strategy find_strategy(int level);
    int generate_identifier_between(int min, int max,strategy _strategy);
    void handle_remote_insert(CRDT_Char Char);
    CRDT_pos find_insert_position(CRDT_Char Char);
    int is_empty();
    CRDT_pos find_end_position (CRDT_Char last_Char, int last_line, int total_lines);
    CRDT_Char find_insert_index_in_line(CRDT_Char Char, int line);
    void insert_Char(CRDT_Char Char, CRDT_pos pos);
};


#endif //UNTITLED_CRDT_H

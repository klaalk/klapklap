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
    void local_insert(char val, kk_pos pos);
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
                                                    vector<kk_identifier_ptr> *new_position, unsigned long livello); //partendo dalle position di due Char(adiacenti) genera la posizione della Char

    strategy find_strategy(unsigned long level);//trova la strategia migliore per assegnare un identifier alla position della nuova Char

    unsigned long generate_identifier_between(unsigned long min, unsigned long max, strategy _strategy); //dati due identifier ne genera uno nuovo da mettere nella position della nuova Char usando la strategia opportuna

    int generate_identifier_between(int min, int max, strategy _strategy);
    void insert_char(kk_char_ptr _char, kk_pos pos);

//    void handle_remote_insert(kk_char _char);
//    kk_pos find_insert_position(kk_char _char);
//    int is_empty();
//    kk_pos find_end_position (kk_char last_char,  list<kk_char> last_line, int total_lines);
//    int find_insert_index_in_line(kk_char _char, list<kk_char> line);

//    void local_delete(kk_pos start_pos, kk_pos end_pos);
//    list<kk_char> delete_multiple_lines(kk_pos start_pos, kk_pos end_pos);
//    list<kk_char> delete_single_line(kk_pos start_pos, kk_pos end_pos);
//    void merge_lines(int line);
//    void remove_empty_lines();
//    void remote_delete(kk_char _Char);
//    kk_pos find_pos (kk_char _Char);
//    int find_index_in_line(kk_char _Char, int line);
    vector<kk_identifier_ptr> slice(vector<kk_identifier_ptr> const &v,int i);
};

typedef std::shared_ptr<kk_crdt> kk_crdt_ptr;
#endif //KK_CRDT_H

//
// Created by Alberto Bruno on 2019-05-15.
//

#include "CRDT_Crdt.h"
#include "CRDT_identifier.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <iostream>

using std::vector;
using std::string;



CRDT_Crdt::CRDT_Crdt(string siteid,int boundary,strategy strategy,int base):siteid(siteid),boundary(10),_strategy(casuale),base(32){};

void CRDT_Crdt::local_insert(char val,CRDT_pos pos ){
    CRDT_Char* new_Char = new CRDT_Char(val, this->siteid);
    *new_Char = this->generate_Char(val,pos);
    this->insert_Char(*new_Char,pos);
    return;

}

CRDT_Char CRDT_Crdt::generate_Char(char val, CRDT_pos pos){

    vector<CRDT_identifier> position_before, position_after, new_position;
    CRDT_Char* new_Char = new CRDT_Char(val, this->siteid);

    position_before=this->find_position_before(pos);
    position_after=this->find_position_after(pos);
    new_position=this->generate_position_between(position_before,position_after,&new_position,0);
    new_Char->insert_position(new_position);
    return *new_Char;
}

vector<CRDT_identifier> CRDT_Crdt::find_position_before(CRDT_pos pos){

    if(pos.get_ch()==0 && pos.get_line()==0){
        vector<CRDT_identifier> vuoto;
        return vuoto;
    }else if(pos.get_ch()==0 && pos.get_line()!=0){
        int line = pos.get_line()-1;
        int ch = this->text[line].size();
        return this->text[line][ch-1].get_position();
    }
    return this->text[pos.get_line()][pos.get_ch()-1].get_position();
}

vector<CRDT_identifier> CRDT_Crdt::find_position_after(CRDT_pos pos){
    int num_lines, num_chars;
    vector<CRDT_identifier> vuoto;

    num_lines = this->text.size();

    if(this->text[pos.get_line()].empty()){
        num_chars=0;
    }else{
        num_chars=this->text[pos.get_line()].size();
    }

    if(pos.get_line()>num_lines && pos.get_ch()==0){ //non dovrebbe mai succedere
        return vuoto;
    }else if(pos.get_line()==num_lines-1 && pos.get_ch()==num_chars){ //sei all'ultima riga e all'ultimo char
        return vuoto;
    }else if(pos.get_line()<num_lines-1 && pos.get_ch()==num_chars){ // sei a fine riga ma non nell'ultima riga
        return this->text[pos.get_line()+1][0].get_position();
    }else{
        return this->text[pos.get_line()][pos.get_ch()].get_position(); // se non sei in nessun caso particolare DA RIVEDERE
    }
}

vector<CRDT_identifier> CRDT_Crdt::generate_position_between(vector<CRDT_identifier> position1,vector<CRDT_identifier> position2,vector<CRDT_identifier> *new_position, int level) {

    strategy _strategy;

    base = pow(2, level) * this->base;
    _strategy = this->find_strategy(level);
    CRDT_identifier *id1, *id2;
    vector <CRDT_identifier> vuoto;

    if (position1.size() == 0) {
        id1 = new CRDT_identifier(0, this->siteid);
    } else {
        *id1 = position1[0];
    }

    if (position2.size() == 0) {
        id1 = new CRDT_identifier(base, this->siteid);
    } else {
        *id1 = position1[0];
    }

    if (id2->get_digit() - id1->get_digit() > 1) {

        int new_digit;
        CRDT_identifier *new_id;

        new_digit = this->generate_identifier_between(id1->get_digit(), *id2->get_digit(), _strategy);
        new_id = new CRDT_identifier(new_digit, this->siteid);
        new_position->insert(new_position->end(), *new_id);
        return *new_position;

    } else if (id2->get_digit() - id1->get_digit() == 1) {

        new_position->insert(new_position->end(), *id1);
        return this->generate_position_between(position1.slice(1),vuoto,&new_position,level+1);

    } else if(id1->get_digit()==id2->get_digit()){
        if(id1->get_siteid()<id2->get_siteid()){
            new_position->insert(new_position->end(), *id1);
            return this->generate_position_between(position1.slice(1),vuoto,&new_position,level+1);

        }else if(id1->get_siteid()==id2->get_siteid()){
            new_position->insert(new_position->end(), *id1);
            return this->generate_position_between(position1.slice(1),position2.slice(1),&new_position,level+1);

        }else{
            //gestire errore
        }

    }
}

strategy CRDT_Crdt::find_strategy(int level){
    if (strategy_cache[level]) {
        return strategy_cache[level];
    }
    strategy _local_strategy;
    switch (_strategy) {
        case plus:
            _local_strategy = plus;
        case minus:
            _local_strategy = minus;
        case casuale:
            _local_strategy = round(rand()) == 0 ? plus : minus;
        default:
            _local_strategy = (level%2) == 0 ? plus : minus;
    }
    this->strategy_cache[level] = _local_strategy;
    return _local_strategy;
}

int CRDT_Crdt::generate_identifier_between(int min, int max, strategy _strategy) {
    if((max-min<this->boundary)){
        min=min+1;
    }else{
        if(_strategy== '-') {
            min = max - this->boundary;
        }else{
            min=min+1;
            max=min + this->boundary;
        }
    }
return floor(rand() * (max - min)) + min;
}

void CRDT_Crdt::handle_remote_insert(CRDT_Char Char){

}

CRDT_pos CRDT_Crdt::find_insert_position(CRDT_Char Char){

}

int CRDT_Crdt::is_empty(){

}

CRDT_pos CRDT_Crdt::find_end_position (CRDT_Char last_Char, int last_line, int total_lines){

}

CRDT_Char CRDT_Crdt::find_insert_index_in_line(CRDT_Char Char, int line){

}

void CRDT_Crdt::insert_Char(CRDT_Char Char, CRDT_pos pos){

}



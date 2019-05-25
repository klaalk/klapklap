//
// Created by Alberto Bruno on 2019-05-15.
//

#include "CRDT_pos.h"

CRDT_pos::CRDT_pos(int line,int ch): line(line), ch(ch) {}

int CRDT_pos::get_ch() {
    return this->ch;
}

int CRDT_pos::get_line() {
    return  this->line;
}
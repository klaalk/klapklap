//
// Created by Alberto Bruno on 2019-05-15.
//

#include "kk_pos.h"

kk_pos::kk_pos(unsigned long line, unsigned long ch) : line(line),ch(ch){}

unsigned long kk_pos::get_ch() {
    return this->ch;
}

unsigned long kk_pos::get_line() {
    return this->line;
}

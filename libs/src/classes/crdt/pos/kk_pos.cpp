//
// Created by Alberto Bruno on 2019-05-15.
//

#include "kk_pos.h"

kk_pos::kk_pos(int line, int ch) : ch(ch), line(line) {}

int kk_pos::get_ch() {
    return this->ch;
}

int kk_pos::get_line() {
    return this->line;
}

//
// Created by Alberto Bruno on 2019-05-15.
//

#include "kk_pos.h"

KKPosition::KKPosition(unsigned long line, unsigned long ch) : line(line),ch(ch)
{

}

unsigned long KKPosition::getCh() {
    return this->ch;
}

unsigned long KKPosition::getLine() {
    return this->line;
}


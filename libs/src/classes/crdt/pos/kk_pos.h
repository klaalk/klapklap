//
// Created by Alberto Bruno on 2019-05-15.
//

#ifndef KK_POS_H
#define KK_POS_H

#include "../../../kklibexport.h"
#include <memory>

class kk_pos {
    unsigned long line;
    unsigned long ch;
public:
    kk_pos();
    kk_pos(unsigned long line, unsigned long ch);
    unsigned long get_line();
    unsigned long get_ch();
};

typedef std::shared_ptr<kk_pos> kk_pos_ptr;
#endif //KK_POS_H

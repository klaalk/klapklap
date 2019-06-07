//
// Created by Alberto Bruno on 2019-05-15.
//

#ifndef KK_POS_H
#define KK_POS_H

#include "../../../kklibexport.h"

class kk_pos {
    int ch;
    int line;
public:
    kk_pos();

    kk_pos(int line, int ch);

    int get_line();

    int get_ch();
};


#endif //KK_POS_H

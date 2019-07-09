//
// Created by Alberto Bruno on 2019-05-15.
//

#ifndef KK_POS_H
#define KK_POS_H

#include "../../../kklibexport.h"
#include <memory>

class KKPosition {
    unsigned long line;
    unsigned long ch;
public:
    KKPosition();
    KKPosition(unsigned long line, unsigned long ch);
    unsigned long getLine();
    unsigned long getCh();
};

typedef std::shared_ptr<KKPosition> KKPosPtr;
#endif //KK_POS_H

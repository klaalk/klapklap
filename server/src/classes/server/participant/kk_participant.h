//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_PARTECIPANT_H
#define KK_PARTECIPANT_H


#include <QString>
#include "../../../../../libs/src/classes/payload/kk_payload.h"

class kk_participant {
public:
    QString nick_;
    virtual ~kk_participant(){};
    virtual void deliver(kk_payload_ptr msg) = 0;
};

#endif //KK_PARTECIPANT_H

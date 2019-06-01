//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_PARTECIPANT_H
#define KK_PARTECIPANT_H

#include <iostream>
#include <deque>
#include<QString>

#include "../../../../../libs/src/classes/payload/kk_payload.h"

class kk_participant {
protected:
    QString name;
public:
    virtual ~kk_participant() {}

    virtual void deliver(const kk_payload &msg) = 0;
};

typedef std::deque<kk_payload> kk_kk_payload_queue;


#endif //KK_PARTECIPANT_H

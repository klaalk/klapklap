//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_PARTECIPANT_H
#define KK_PARTECIPANT_H


#include <QString>
#include "../../../../../libs/src/classes/payload/kk_payload.h"

class KKParticipant {
public:
    QString id = "Unknown";
    virtual void deliver(KKPayloadPtr msg) = 0;
};
typedef QSharedPointer<KKParticipant> KKParticipantPtr;

#endif //KK_PARTECIPANT_H

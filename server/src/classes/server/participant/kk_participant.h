//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_PARTECIPANT_H
#define KK_PARTECIPANT_H


#include <QString>
#include "../../../../../libs/src/classes/payload/kk_payload.h"

class KKParticipant {
public:
    KKParticipant();
    virtual ~KKParticipant();

    QString id = "unknown";
    virtual void deliver(KKPayload msg) = 0;
};
typedef QSharedPointer<KKParticipant> KKParticipantPtr;
typedef QSharedPointer<QMap<QString, KKParticipantPtr>> KKMapParticipantPtr;

#endif //KK_PARTECIPANT_H

//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_ROOM_H
#define KK_ROOM_H

#include <set>
#include <algorithm>
#include <memory>
#include "../../../../../libs/src/classes/payload/kk_payload.h"
#include "../participant/kk_participant.h"
#include <QSharedPointer>
class KKRoom {
public:
    void join(QSharedPointer<KKParticipant> participant);

    void leave(QSharedPointer<KKParticipant> participant);

private:
    std::set<QSharedPointer<KKParticipant>> participants;
};

#endif //KK_ROOM_H

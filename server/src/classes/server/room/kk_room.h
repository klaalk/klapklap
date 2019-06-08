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
class kk_room {
public:
    void join(QSharedPointer<kk_participant> participant);

    void leave(QSharedPointer<kk_participant> participant);

private:
    std::set<QSharedPointer<kk_participant>> participants_;
};

#endif //KK_ROOM_H

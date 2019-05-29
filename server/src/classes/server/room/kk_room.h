//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_ROOM_H
#define KK_ROOM_H

#include <set>
#include <algorithm>
#include <boost/bind.hpp>
#include "../../../../../libs/src/classes/payload/kk_payload.h"
#include "../partecipant/kk_partecipant.h"

class kk_room {
public:
    void join(kk_participant_ptr participant);

    void leave(kk_participant_ptr participant);

private:
    std::set<kk_participant_ptr> participants_;
};

#endif //KK_ROOM_H

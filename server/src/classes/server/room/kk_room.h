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

class kk_room {
public:
    void join(std::shared_ptr<kk_participant> participant);

    void leave(std::shared_ptr<kk_participant> participant);

private:
    std::set<std::shared_ptr<kk_participant>> participants_;
};

#endif //KK_ROOM_H

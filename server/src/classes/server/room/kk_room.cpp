//
// Created by Klaus on 06/05/2019.
//

#include "kk_room.h"

void kk_room::join(std::shared_ptr<kk_participant> participant) {
    participants_.insert(participant);
}

void kk_room::leave(std::shared_ptr<kk_participant> participant) {
    participants_.erase(participant);
}

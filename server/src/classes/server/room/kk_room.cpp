//
// Created by Klaus on 06/05/2019.
//

#include "kk_room.h"

void kk_room::join(kk_participant_ptr participant) {
    participants_.insert(participant);
}

void kk_room::leave(kk_participant_ptr participant) {
    participants_.erase(participant);
}

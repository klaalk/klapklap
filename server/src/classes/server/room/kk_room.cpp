//
// Created by Klaus on 06/05/2019.
//

#include "kk_room.h"

void kk_room::join(QSharedPointer<kk_participant> participant) {
    participants_.insert(participant);
}

void kk_room::leave(QSharedPointer<kk_participant> participant) {
    participants_.erase(participant);
}

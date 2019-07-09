//
// Created by Klaus on 06/05/2019.
//

#include "kk_room.h"

void KKRoom::join(QSharedPointer<KKParticipant> participant) {
    participants.insert(participant);
}

void KKRoom::leave(QSharedPointer<KKParticipant> participant) {
    participants.erase(participant);
}

//
// Created by Klaus on 06/05/2019.
//

#include "crdt_room.h"

void crdt_room::join(crdt_participant_ptr participant)
{
    participants_.insert(participant);
}

void crdt_room::leave(crdt_participant_ptr participant)
{
    participants_.erase(participant);
}

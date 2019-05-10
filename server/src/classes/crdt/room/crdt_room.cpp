//
// Created by Klaus on 06/05/2019.
//

#include "crdt_room.h"

void crdt_room::join(crdt_participant_ptr participant)
{
    participants_.insert(participant);
    std::for_each(recent_msgs_.begin(), recent_msgs_.end(),
                  boost::bind(&crdt_participant::deliver, participant, _1));
}

void crdt_room::leave(crdt_participant_ptr participant)
{
    participants_.erase(participant);
}

void crdt_room::deliver(const chat_message& msg)
{
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
        recent_msgs_.pop_front();

    std::for_each(participants_.begin(), participants_.end(),
                  boost::bind(&crdt_participant::deliver, _1, boost::ref(msg)));
}

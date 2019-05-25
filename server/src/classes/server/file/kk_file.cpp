//
// Created by Klaus on 06/05/2019.
//

#include "kk_file.h"

void kk_file::join(kk_participant_ptr participant)
{
    participants_.insert(participant);
    std::for_each(recent_msgs_.begin(), recent_msgs_.end(),
                  boost::bind(&kk_participant::deliver, participant, _1));
    std::cout << "partecipante inserito correttamente" << std::endl;
}

void kk_file::leave(kk_participant_ptr participant)
{
    participants_.erase(participant);
}

void kk_file::deliver(const kk_payload& msg)
{
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
        recent_msgs_.pop_front();

    std::for_each(participants_.begin(), participants_.end(),
                  boost::bind(&kk_participant::deliver, _1, boost::ref(msg)));
}

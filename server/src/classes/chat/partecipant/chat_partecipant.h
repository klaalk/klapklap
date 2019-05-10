//
// Created by Klaus on 06/05/2019.
//

#ifndef SERVER_CHAT_PARTECIPANT_H
#define SERVER_CHAT_PARTECIPANT_H
#include <boost/asio.hpp>
#include <deque>

#include "../../../../../libs/src/classes/chat/message/chat_message.h"

class chat_participant
{
public:
    virtual ~chat_participant() {}
    virtual void deliver(const chat_message& msg) = 0;
};

typedef boost::shared_ptr<chat_participant> chat_participant_ptr;
typedef std::deque<chat_message> chat_message_queue;

#endif //SERVER_CHAT_PARTECIPANT_H

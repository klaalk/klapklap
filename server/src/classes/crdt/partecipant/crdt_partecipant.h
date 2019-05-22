//
// Created by Klaus on 06/05/2019.
//

#ifndef SERVER_CRDT_PARTECIPANT_H
#define SERVER_CRDT_PARTECIPANT_H
#include <boost/asio.hpp>
#include <deque>

#include "../../../../../libs/src/classes/message/message.h"

class crdt_participant
{
protected:
    std::string name;
public:
    virtual ~crdt_participant() {}
    virtual void deliver(const message& msg) = 0;
};

typedef boost::shared_ptr<crdt_participant> crdt_participant_ptr;
typedef std::deque<message> crdt_message_queue;


#endif //SERVER_CRDT_PARTECIPANT_H

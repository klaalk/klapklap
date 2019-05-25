//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_PARTECIPANT_H
#define KK_PARTECIPANT_H
#include <boost/asio.hpp>
#include <deque>

#include "../../../../../libs/src/classes/payload/kk_payload.h"

class kk_participant
{
protected:
    std::string name;
public:
    virtual ~kk_participant() {}
    virtual void deliver(const kk_payload& msg) = 0;
};

typedef boost::shared_ptr<kk_participant> kk_participant_ptr;
typedef std::deque<kk_payload> kk_kk_payload_queue;


#endif //KK_PARTECIPANT_H

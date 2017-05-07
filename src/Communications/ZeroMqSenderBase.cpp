// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#include "ZeroMqSenderBase.h"

#include "ZeroMqFabric.h"
#include "UxAS_Log.h"

#include <chrono>
#include <thread>

namespace uxas
{
namespace communications
{
namespace transport
{

ZeroMqSenderBase::~ZeroMqSenderBase()
{
//    LOG_INFORM_ASSIGNMENT("~ZeroMqSenderBase() -Begin");
    uint32_t lingerDuration_ms(0);
    m_zmqSocket->setsockopt(ZMQ_LINGER, &lingerDuration_ms, sizeof (lingerDuration_ms));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    m_zmqSocket->close();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    m_zmqSocket.reset();
//    LOG_INFORM_ASSIGNMENT("~ZeroMqSenderBase()- End");
};

void
ZeroMqSenderBase::initialize(const std::string& sourceGroup, uint32_t entityId, uint32_t serviceId, SocketConfiguration& zeroMqSocketConfiguration)
{
    m_sourceGroup = sourceGroup;
    m_entityId = entityId;
    m_serviceId = serviceId;
    m_entityIdString = std::to_string(entityId);
    m_serviceIdString = std::to_string(serviceId);

    m_zeroMqSocketConfiguration = static_cast<ZeroMqSocketConfiguration&> (zeroMqSocketConfiguration);
    m_zmqSocket = ZeroMqFabric::getInstance().createSocket(m_zeroMqSocketConfiguration);
};

}; //namespace transport
}; //namespace communications
}; //namespace uxas
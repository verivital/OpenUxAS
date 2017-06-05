// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

/* 
 * File:   GamsService.cpp
 * Author: James Edmondson <jedmondson@gmail.com>
 *
 * Created on May 30, 2017, 3:40 PM
 *
 */


#include "GamsService.h"

#include "UnitConversions.h"
#include "UxAS_TimerManager.h"

#include "afrl/cmasi/AirVehicleState.h"
#include "afrl/cmasi/AutomationResponse.h"
#include "afrl/cmasi/GimbalAngleAction.h"
#include "afrl/cmasi/LoiterAction.h"
#include "uxas/messages/uxnative/IncrementWaypoint.h"

#include "uxas/madara/MadaraState.h"

#include "pugixml.hpp"

#include <iostream>

#define STRING_COMPONENT_NAME "GamsService"

#define STRING_XML_COMPONENT "Component"
#define STRING_XML_TYPE "Type"
#define STRING_XML_COMPONENT_TYPE "GamsService"
#define STRING_XML_VEHICLE_ID "VehicleID"


#define COUT_INFO(MESSAGE) std::cout << "<>GamsService:: " << MESSAGE << std::endl;std::cout.flush();
#define COUT_FILE_LINE_MSG(MESSAGE) std::cout << "<>GamsService:: " << __FILE__ << ":" << __LINE__ << ":" << MESSAGE << std::endl;std::cout.flush();
#define CERR_FILE_LINE_MSG(MESSAGE) std::cerr << "<>GamsService:: " << __FILE__ << ":" << __LINE__ << ":" << MESSAGE << std::endl;std::cout.flush();

namespace knowledge = madara::knowledge;
namespace transport = madara::transport;


knowledge::KnowledgeBase uxas::service::GamsService::s_knowledgeBase;



namespace uxas
{
namespace service
{
  /**
   * Transport for UxAS GamsService to convert knowledge to UxAS Messages
   **/
  class UxASMadaraTransport : public transport::Base
  {
  public:
      /**
       * Constructor
       * @param   id                unique identifier (generally host:port)
       * @param   new_settings      settings to apply to the transport
       * @param   context           the knowledge record context
       * @param   service           the calling GamsService
       **/
      UxASMadaraTransport (const std::string & id,
        transport::TransportSettings & new_settings,
        knowledge::KnowledgeBase & knowledge,
        GamsService * service)
      : transport::Base (id, new_settings, knowledge.get_context ()),
        m_service (service)
      {
        // populate variables like buffer_ based on transport settings
        transport::Base::setup ();
      }

      /**
       * Destructor
       **/
      virtual ~UxASMadaraTransport ()
      {

      }
    
      /**
       * Sends a list of updates to the UxAS system
       * @param  modifieds  a list of keys to values of all records that have
       *          been updated and could be sent.
       * @return  result of operation or -1 if we are shutting down
       **/
      virtual long send_data (const knowledge::KnowledgeRecords & modifieds)
      {
        ///Return number of bytes sent or negative for error
        long result (0);
        
        /// Prefix for printing purposes in madara logger
        const char * printPrefix = "UxASMadaraTransport::send_data";

        /// prepare the buffer for sending by filling in all modifieds
        result = prep_send (modifieds, printPrefix);

        /// if the service exists (which it should), send the MadaraState
        if (m_service)
        {
            m_service->sendBuffer(buffer_.get (), (size_t)result);
            
            /// add the bytes sent to the send bandwidth monitor
            send_monitor_.add ((uint32_t)result);
        }

        /// return the buffer size that was sent
        return result;
      }
    
  protected:
      
      /// handle to the GamsService so we can use sendBuffer
      GamsService * m_service;
  };
    
    
    
GamsService::ServiceBase::CreationRegistrar<GamsService>
GamsService::s_registrar(GamsService::s_registryServiceTypeNames());

GamsService::GamsService()
: ServiceBase(GamsService::s_typeName(), GamsService::s_directoryName()) { };

GamsService::~GamsService() { };

bool
GamsService::configure(const pugi::xml_node& ndComponent)
{
    // in the future, XML parameters to configure new transports
    // and load algorithms
    
    // attach the MadaraTransport for knowledge modifications to UxAS messages
    s_knowledgeBase.attach_transport (
      new UxASMadaraTransport (m_uniqueId, m_transportSettings, s_knowledgeBase,
            this));
    
    
    addSubscriptionAddress(afrl::cmasi::AirVehicleState::Subscription);
    addSubscriptionAddress(afrl::impact::GroundVehicleState::Subscription);
    addSubscriptionAddress(uxas::madara::MadaraState::Subscription);
    
    return true;
}

bool
GamsService::initialize()
{
    bool bSuccess(true);

    return (bSuccess);
};

bool
GamsService::terminate()
{

    return true;
}


void
GamsService::sendBuffer (char * buffer, size_t length)
{
    uxas::madara::MadaraState * newMessage = new uxas::madara::MadaraState ();
    
    // @DEREK
    // MadaraState has declared __Contents as protected and there is no
    // setter. How am I supposed to get my buffer to the MadaraState message?
    // In KeyValue pair, there appears to be a generated setter. How do I get
    // this for a byte array as we do in MadaraState?
    
    std::vector <uint8_t> contents;
    contents.resize (length);
    for (size_t i = 0; i < length; ++i)
    {
        contents[i] = (uint8_t)buffer[i];
    }
    
    // we should be able to set the contents, otherwise, no way to create
    // message
    newMessage->setContents (contents);
    
    this->sendLmcpObjectBroadcastMessage(newMessage);
}

bool
GamsService::processReceivedLmcpMessage(std::unique_ptr<uxas::communications::data::LmcpMessage> receivedLmcpMessage)
{
    if (afrl::cmasi::isAirVehicleState(receivedLmcpMessage->m_object.get()))
    {
        // we do not currently process this
    }
    else if (afrl::impact::isGroundVehicleState(receivedLmcpMessage->m_object.get()))
    {
        // we do not currently process this
    }
    else if (uxas::madara::isMadaraState (receivedLmcpMessage->m_object.get()))
    {
        // clone the LMCP message into a MadaraState structure
        std::shared_ptr<uxas::madara::MadaraState> ptr_MadaraState(
            static_cast<uxas::madara::MadaraState *> (
                receivedLmcpMessage->m_object.get()));
        
        
        // contents are returned as a byte vector. Convert that into a char[]
        std::vector<uint8_t> bufferVector = ptr_MadaraState->getContents ();
        
        char * buffer = new char [bufferVector.size ()];
        if (buffer)
        {
            for (size_t i = 0; i < bufferVector.size (); ++i)
            {
                buffer[i] = (char) bufferVector[i];
            }
        }
        
        // keep track of records for rebroadcast, but don't do so (yet)
        knowledge::KnowledgeMap rebroadcastRecords;
        
        // if we want to look at the MADARA message header after
        transport::MessageHeader * header = 0;
        
        // a print prefix for logging
        const char * printPrefix = "GamsService::processReceivedLmcpMessage";
        
        transport::process_received_update (buffer,
            (ssize_t)bufferVector.size (),
            m_uniqueId,
            *m_context,
            m_transportSettings,
            m_sendMonitor, m_receiveMonitor, rebroadcastRecords,
            m_onDataReceived,
            printPrefix,
            receivedLmcpMessage->m_attributes->getSourceEntityId ().c_str (),
            header);
    }

    return false;
};

}; //namespace service
}; //namespace uxas
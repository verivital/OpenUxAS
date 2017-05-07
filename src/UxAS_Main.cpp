// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#include "afrl/cmasi/AirVehicleState.h"
#include "afrl/impact/GroundVehicleState.h"

#include "LmcpObjectNetworkBridgeManager.h"
#include "LmcpObjectNetworkServer.h"
#include "ServiceManager.h"
#include "ZeroMqFabric.h"

#include "UxAS_ConfigurationManager.h"
#include "UxAS_ConsoleLogger.h"
#include "UxAS_FileLogger.h"
#include "UxAS_Log.h"
#include "UxAS_LogManagerDefaultInitializer.h"
#include "UxAS_StringUtil.h"

#ifdef AFRL_INTERNAL_ENABLED
#include "UxAS_SerialPortEmulator.h"
#endif

#include "stdUniquePtr.h"

#include "FileSystemUtilities.h"

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <locale>

#define ARG_CFG_PATH "-cfgPath"
#define ARG_VERSION "-version"

#define MAJOR_VERSION "3"
#define MINOR_VERSION "1"
#define PATCH_VERSION "0"

#define BEFORE_LOG_MANAGER_INITIALIZATION_LOG_MESSAGE(message) std::cout << message << std::endl; std::cout.flush();

int
main(int argc, char** argv)
{
    // override locale with US locale settings
    /*
    std::locale loc;
    try
    {
        loc.global(std::locale("en_US.UTF8"));
    }
    catch (std::runtime_error)
    {
        std::cerr << "main: " << "Could not set locale to US !" << std::endl;
    }
     */

    // 
    // declare relative paths of configuration files with default values
    // example arguments: -cfgBasePath ./cfg/cfgbase2.xml
    //
    std::string cfgPath{"cfg.xml"};

    for (int i = 1; i < argc; i++)
    {
        if (strcmp((const char *) argv[i], ARG_CFG_PATH) == 0)
        {
            i++;
            cfgPath = std::string(argv[i]);
        }
        else if (strcmp((const char *) argv[i], ARG_VERSION) == 0)
        {
            auto o = new afrl::cmasi::AirVehicleState;
            auto p = new afrl::impact::GroundVehicleState;
            std::cout << std::endl << "#######################" << std::endl;
            std::cout << "   VERSION: " << MAJOR_VERSION << "." << MINOR_VERSION << "." << PATCH_VERSION << std::endl;
            std::cout << "     CMASI:  " << o->getSeriesVersion() << std::endl;
            std::cout << "     IMPACT: " << p->getSeriesVersion() << std::endl;
            std::cout << "#######################" << std::endl << std::endl;
            delete o;
            delete p;
        }
        else
        {
            std::cerr << "Unrecognized argument " << argv[i] << std::endl;
            std::cerr.flush();
            return -1;
        }
    }

    //
    // logging
    //
    bool isConsoleLoggerInitialized = uxas::common::log::LogManagerDefaultInitializer::initializeConsoleLogger();
    if (isConsoleLoggerInitialized)
    {
        LOG_INFORM("UxAS_Main initialized console logger");
    }
    else
    {
        BEFORE_LOG_MANAGER_INITIALIZATION_LOG_MESSAGE("ERROR UxAS_Main failed to initialize console logger")
    }

    //    bool isDatabaseLoggerInitialized = uxas::common::log::LogManagerDefaultInitializer::initializeMainDatabaseLogger();
    //    if (isDatabaseLoggerInitialized)
    //    {
    //        LOG_INFORM("UxAS_Main initialized main file logger");
    //    }
    //    else
    //    {
    //        BEFORE_LOG_MANAGER_INITIALIZATION_LOG_MESSAGE("ERROR UxAS_Main failed to initialize main database logger")
    //    }

    bool isMainFileLoggerInitialized = uxas::common::log::LogManagerDefaultInitializer::initializeMainFileLogger();
    if (isMainFileLoggerInitialized)
    {
        LOG_INFORM("UxAS_Main initialized main file logger");
    }
    else
    {
        BEFORE_LOG_MANAGER_INITIALIZATION_LOG_MESSAGE("ERROR UxAS_Main failed to initialize main file logger")
    }

    //
    // log thread id
    //
    LOG_INFORM("UxAS_Main running on main thread [", std::this_thread::get_id(), "]");

    /* NOTES:
     * (a) Configurations are determined from two files - details:
     *     - base XML containing default values
     *     - extension XML containing override values
     *     - services and bridges found in extension XML are instantiated (otherwise, not created)
     *     - XML structure is same for Service and Bridge nodes as compared to Component nodes
     * (b) Messaging is either multi-part or single-part as determined by hard-coded
     *     value in ConfigurationManager.
     * (c) Messages consist of seven fields:
     *     - address (notional example values: "uxas.project.isolate.IntruderAlert", "eId12sId14", "uxas.roadmonitor")
     *     - contentType (e.g., "lmcp", "json", "xml")
     *     - descriptor (e.g., "afrl.cmasi.AirVehicleState" if contentType="lmcp" or a json content descriptor; intent is some flexibility on values depending on contentType)
     *     - senderGroup (notional example values: "fusion", "fusion.operator.sensor", "uxas", "agent", "uxas.roadmonitor")
     *     - senderEntityId
     *     - senderServiceId
     *     - payload
     * (d) Services and bridges use the same socket addresses as components
     *     - static const std::string& strGetInProc_FromMessageHub(){static std::string strString("inproc://from_message_hub");return(strString);};
     *     - static const std::string& strGetInProc_ToMessageHub(){static std::string strString("inproc://to_message_hub");return(strString);};
     * 
     */

    //
    // configuration manager
    //
    if (uxas::common::ConfigurationManager::getInstance().loadBaseXmlFile(cfgPath))
    {
        LOG_INFORM("UxAS_Main loaded base XML configuration from [", cfgPath, "]");
    }
    else
    {
        LOG_ERROR("UxAS_Main failed to load base XML configuration from [", cfgPath, "]");
        return (100);
    }

    //
    // internal message network server
    //
    auto networkServer = uxas::stduxas::make_unique<uxas::communications::LmcpObjectNetworkServer>();

    if (networkServer)
    {
        LOG_INFORM("UxAS_Main created networkServer");
    }
    else
    {
        LOG_ERROR("UxAS_Main failed to create networkServer");
        return (200);
    }

    if (networkServer->configure())
    {
        LOG_INFORM("UxAS_Main configured networkServer");
    }
    else
    {
        LOG_ERROR("UxAS_Main failed to configure networkServer");
        return (210);
    }

    if (networkServer->initializeAndStart())
    {
        LOG_INFORM("UxAS_Main initialized and started networkServer");
    }
    else
    {
        LOG_ERROR("UxAS_Main failed to initialize and start networkServer");
        return (220);
    }

    //
    // build bridges
    //
    if (uxas::communications::LmcpObjectNetworkBridgeManager::getInstance().initialize())
    {
        LOG_INFORM("UxAS_Main initialized bridge manager");
    }
    else
    {
        LOG_ERROR("UxAS_Main failed to initialize bridge manager");
        return (300);
    }

    //
    // service manager and services
    //
    if (uxas::service::ServiceManager::getInstance().configureServiceManager())
    {
        LOG_INFORM("UxAS_Main configured ServiceManager");
    }
    else
    {
        LOG_ERROR("UxAS_Main failed to configure ServiceManager");
        return (400);
    }

    if (uxas::service::ServiceManager::getInstance().initializeAndStartService())
    {
        LOG_INFORM("UxAS_Main initialized and started ServiceManager");
    }
    else
    {
        LOG_ERROR("UxAS_Main failed to initialize and start ServiceManager");
        return (410);
    }

    LOG_INFORM("UxAS_Main running ServiceManager");
    uxas::service::ServiceManager::getInstance().runUntil(uxas::common::ConfigurationManager::getInstance().getRunDuration_s());

    uxas::service::ServiceManager::getInstance().destroyServiceManager();
    networkServer->terminate();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    networkServer.reset();
    uxas::communications::transport::ZeroMqFabric::getInstance().~ZeroMqFabric();

    std::cout << std::endl << "***************************************************" << std::endl << std::endl;
    std::cout << std::endl << "****** UxAS is Shutting Down immediately !!! ******" << std::endl << std::endl;
    std::cout << std::endl << "***************************************************" << std::endl << std::endl;

    //this causes a nasty termination
    // but it terminates. there probably a dangling thread, somewhere
    try 
    {
        throw; // Insert code that will return by throwing a exception.
    }
    catch (const std::exception& e) // Consider using a custom exception type for intentional
    { // throws. A good idea might be a `return_exception`.
        //        return EXIT_SUCCESS;
        std::cout << "Standard exception: " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;


};
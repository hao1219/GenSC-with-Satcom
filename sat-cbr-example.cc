/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 *
 */

#include "ns3/core-module.h"              
#include "ns3/network-module.h"           
#include "ns3/internet-module.h"          
#include "ns3/satellite-module.h"       
#include "ns3/propagation-module.h"       
#include "ns3/applications-module.h"      
#include "ns3/config-store-module.h"    
#include <sys/stat.h>                   
#include <sys/types.h>                    
using namespace ns3;

/**
 * \file sat-cbr-example.cc
 * \ingroup satellite
 *
 * \brief  Cbr example application to use satellite network.
 *          Interval, packet size and test scenario can be given
 *         in command line as user argument.
 *         To see help for user arguments:
 *         execute command -> ./waf --run "sat-cbr-example --PrintHelp"
 *
 *         Cbr example application sends first packets from GW connected user
 *         to UT connected users and after that from UT connected user to GW connected
 *         user.
 *
 */

NS_LOG_COMPONENT_DEFINE ("sat-cbr-example");



void ReceivePacket(Ptr<Socket> socket) {
    Ptr<Packet> packet;
    while ((packet = socket->Recv())) {
        uint8_t buffer[1024];
        packet->CopyData(buffer, packet->GetSize());
        std::string receivedMessage(reinterpret_cast<char*>(buffer), packet->GetSize());

        // Write the received message to output.bin
        std::ofstream outputFile("contrib/satellite/examples/output.bin", std::ios::binary | std::ios::app);
        if (outputFile.is_open()) {
            outputFile.write(reinterpret_cast<const char*>(buffer), packet->GetSize());
            outputFile.close();
            //NS_LOG_INFO("Received message written to output.bin");
        } else {
            NS_LOG_ERROR("Failed to open output.bin for writing");
        }

        //NS_LOG_INFO("Received message: " << receivedMessage);
    }
}
// Function to read binary file and return its content as a string
std::string ReadBinaryFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        NS_LOG_ERROR("Failed to open file: " << filepath);
        return "";
    }

    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::string message(buffer.begin(), buffer.end());
    NS_LOG_INFO("Read binary message: " << message);
    return message;
}
class UtToGatewayHandler {
public:
    UtToGatewayHandler(Ptr<SatHelper> helper, Ptr<Node> utNode, Ptr<Node> gwNode, uint16_t port)
        : m_helper(helper), m_utNode(utNode), m_gwNode(gwNode), m_port(port) {}

    void HandlePacket(Ptr<Socket> socket) {
        Ptr<Packet> packet;
        while ((packet = socket->Recv())) {
            NS_LOG_INFO("UT received packet of size: " << packet->GetSize());
            //NS_LOG_INFO("UT Node: " << m_utNode);
            //NS_LOG_INFO("UdpSocketFactory TypeId: " << UdpSocketFactory::GetTypeId());

            Ptr<Socket> utToGwSocket = Socket::CreateSocket(m_utNode, UdpSocketFactory::GetTypeId());
            utToGwSocket->Connect(InetSocketAddress(m_helper->GetUserAddress(m_gwNode), m_port));
            utToGwSocket->Send(packet);
            NS_LOG_INFO("UT forwarded packet to Gateway");
        }
    }

private:
    Ptr<SatHelper> m_helper;
    Ptr<Node> m_utNode;
    Ptr<Node> m_gwNode;
    uint16_t m_port;
};

class GwToUserHandler {
public:
    GwToUserHandler(Ptr<SatHelper> helper, Ptr<Node> gwNode, Ptr<Node> userNode, uint16_t port)
        : m_helper(helper), m_gwNode(gwNode), m_userNode(userNode), m_port(port) {}

    void HandlePacket(Ptr<Socket> socket) {
        Ptr<Packet> packet;
        while ((packet = socket->Recv())) {
            NS_LOG_INFO("Gateway received packet of size: " << packet->GetSize());
            Ptr<Socket> gwToUserSocket = Socket::CreateSocket(m_gwNode, UdpSocketFactory::GetTypeId());
            gwToUserSocket->Connect(InetSocketAddress(m_helper->GetUserAddress(m_userNode), m_port));
            gwToUserSocket->Send(packet);
            NS_LOG_INFO("Gateway forwarded packet to User");
        }
    }

private:
    Ptr<SatHelper> m_helper;
    Ptr<Node> m_gwNode;
    Ptr<Node> m_userNode;
    uint16_t m_port;
};


class UserReceiveHandler {
public:
    UserReceiveHandler(uint16_t port) : m_port(port), total_received_size(0) {

        std::string directory = "contrib/satellite/examples";
        struct stat info;
        if (stat(directory.c_str(), &info) != 0) {

            if (mkdir(directory.c_str(), 0775) == 0) {
                NS_LOG_INFO("Successfully created directory: " << directory);
            } else {
                NS_LOG_ERROR("Failed to create directory: " << directory);
            }
        } else if (!(info.st_mode & S_IFDIR)) {
            NS_LOG_ERROR("Path exists but is not a directory: " << directory);
        }
    }

    void HandlePacket(Ptr<Socket> socket) {
        Ptr<Packet> packet;
        static std::vector<uint8_t> full_buffer;  

        while ((packet = socket->Recv())) {
            uint8_t buffer[10000];
            size_t packet_size = packet->CopyData(buffer, packet->GetSize());
            total_received_size += packet_size;

            NS_LOG_INFO("User received packet of size: " << packet_size);
            full_buffer.insert(full_buffer.end(), buffer, buffer + packet_size);


            if (full_buffer.size() >= 3072) {
                WriteToFile("contrib/satellite/examples/output.bin", full_buffer);
                full_buffer.clear();  
            }
            //NS_LOG_INFO("Total received size: " << total_received_size << " bytes");
        }
    }

private:
    uint16_t m_port;
    size_t total_received_size;

    void WriteToFile(const std::string& filename, const std::vector<uint8_t>& data) {
        std::ofstream outputFile(filename, std::ios::binary | std::ios::app);
        if (!outputFile.is_open()) {
            NS_LOG_ERROR("Failed to open " << filename << " for writing.");
        } else {
            outputFile.write(reinterpret_cast<const char*>(data.data()), data.size());
            outputFile.close();
            //NS_LOG_INFO("Written " << data.size() << " bytes to " << filename);
        }
        LogFileSize(filename);
    }

    void LogFileSize(const std::string& filename) {
        struct stat fileStat;
        if (stat(filename.c_str(), &fileStat) == 0) {
            //NS_LOG_INFO("Current size of " << filename << ": " << fileStat.st_size << " bytes");
        } else {
            NS_LOG_ERROR("Failed to retrieve file size for " << filename);
        }
    }
};


int
main (int argc, char *argv[])
{
  uint32_t beamIdInFullScenario = 10;
  uint32_t packetSize = 512;
  std::string interval = "1s";
  std::string scenario = "full";
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;

  std::string binaryFilePath = "contrib/satellite/examples/encode_msg.bin";
  std::string customMessage = ReadBinaryFile(binaryFilePath);

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-cbr");


  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("beamIdInFullScenario", "Id where Sending/Receiving UT is selected in FULL scenario. (used only when scenario is full) ", beamIdInFullScenario);
  cmd.AddValue ("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue ("interval", "Interval to sent packets in seconds, (e.g. (1s)", interval);
  cmd.AddValue ("scenario", "Test scenario to use. (simple, larger or full", scenario);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);
  // Enable logs

  if ( scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if ( scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }
  // Set tag, if output path is not explicitly defined
  simulationHelper->SetOutputTag (scenario);
  simulationHelper->SetSimulationTime (Seconds (11));

  // Set beam ID
  std::stringstream beamsEnabled;
  beamsEnabled << beamIdInFullScenario;
  simulationHelper->SetBeams (beamsEnabled.str ());


  // enable info logs
  LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("sat-cbr-example", LOG_LEVEL_INFO);
  if (customMessage.empty()) {
      NS_LOG_INFO("Exiting: Failed to read binary file or file is empty.");
      return 1;
  }
  
  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario (satScenario);

  if ( scenario == "full")
    {
      // Manual configuration of applications

      // get users
    NodeContainer uts = helper->GetBeamHelper ()->GetUtNodes (0, beamIdInFullScenario);
    NodeContainer utUsers = helper->GetUserHelper ()->GetUtUsers (uts.Get (0));
    NodeContainer gwUsers = helper->GetGwUsers ();

    uint16_t port = 9;
    // Step 1: User -> UT
    Ptr<Socket> userSocket = Socket::CreateSocket(utUsers.Get(0), UdpSocketFactory::GetTypeId());
    userSocket->Connect(InetSocketAddress(helper->GetUserAddress(uts.Get(0)), port));
    Simulator::Schedule(Seconds(2.0), [userSocket,customMessage]() {
        //std::string message = "Hello, this is a custom message!";
        Ptr<Packet> customPacket = Create<Packet>((uint8_t*)customMessage.c_str(), customMessage.size());
        userSocket->Send(customPacket);
        NS_LOG_INFO("User sent custom packet to UT");
    });
  
    // Step 2: UT -> Satellite -> GW
       
    UtToGatewayHandler utHandler(helper, uts.Get(0), gwUsers.Get(0), port);
    Ptr<Socket> utSocket = Socket::CreateSocket(uts.Get(0), UdpSocketFactory::GetTypeId());
    utSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), port));
    utSocket->SetRecvCallback(MakeCallback(&UtToGatewayHandler::HandlePacket, &utHandler));

    // Step 3: GW -> Another User 
    GwToUserHandler gwHandler(helper, gwUsers.Get(0), utUsers.Get(1), port);
    Ptr<Socket> gwSocket = Socket::CreateSocket(gwUsers.Get(0), UdpSocketFactory::GetTypeId());
    gwSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), port));
    gwSocket->SetRecvCallback(MakeCallback(&GwToUserHandler::HandlePacket, &gwHandler));

   // Step 4: user callback
    UserReceiveHandler userReceiveHandler(port);
    Ptr<Socket> userReceiveSocket = Socket::CreateSocket(utUsers.Get(1), UdpSocketFactory::GetTypeId());
    userReceiveSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), port));
    userReceiveSocket->SetRecvCallback(MakeCallback(&UserReceiveHandler::HandlePacket, &userReceiveHandler));


    };

  NS_LOG_INFO ("Scenario used: " << scenario);
  if ( scenario == "full" )
    {
      NS_LOG_INFO ("UT used in full scenario from beam: " << beamIdInFullScenario );
    }
  //NS_LOG_INFO ("  PacketSize: " << packetSize);
  //NS_LOG_INFO ("  Interval: " << interval);
  //NS_LOG_INFO ("  ");
  simulationHelper->RunSimulation ();

  return 0;
}

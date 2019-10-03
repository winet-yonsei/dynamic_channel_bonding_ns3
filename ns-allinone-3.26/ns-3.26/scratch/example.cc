/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 SEBASTIEN DERONNE
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
 * Author: Sebastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"

// This code based on example/vht-wifi-network.cc 

// This is a simple example in order to show how to configure an IEEE 802.11ac Wi-Fi network.
//
//  STA     AP
//    *     *
//    |     |
//   n1     n2
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("vht-wifi-network");

int main (int argc, char *argv[])
{
	bool udp = true;
	double simulationTime = 10; //seconds
	double distance = 1.0; //meters

	CommandLine cmd;
	cmd.AddValue("distance", "Distance in meters between the station and the access point", distance);
	cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
	cmd.AddValue("udp", "UDP if set to 1, TCP otherwise", udp);
	cmd.Parse(argc, argv);

	std::cout << "Channel width" << "\t\t" << "Throughput" << '\n';



	uint32_t payloadSize; //1500 byte IP packet
	if (udp)
	{
		payloadSize = 1472; //bytes
	}
	else
	{
		payloadSize = 1448; //bytes
		Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));
	}

	NodeContainer wifiStaNode, wifiStaNode2;
	wifiStaNode.Create(1); wifiStaNode2.Create(1);
	NodeContainer wifiApNode, wifiApNode2;
	wifiApNode.Create(1); wifiApNode2.Create(1);

	YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
	phy.SetChannel(channel.Create());

	// Set guard interv58374 Mbit/s

	phy.Set("ShortGuardEnabled", BooleanValue(0));

	WifiHelper wifi;
	wifi.SetStandard(WIFI_PHY_STANDARD_80211ac);
	WifiMacHelper mac;

	wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager", "RtsCtsThreshold", UintegerValue(100),
		"PacketLength", UintegerValue(payloadSize));

	NetDeviceContainer staDevice, staDevice2;
	NetDeviceContainer apDevice, apDevice2;


	// make 1st ap-sta pair
	Ssid ssid = Ssid("ns3-80211ac");

	mac.SetType("ns3::StaWifiMac",
		"Ssid", SsidValue(ssid));
	
	staDevice = wifi.Install(phy, mac, wifiStaNode);

	mac.SetType("ns3::ApWifiMac",
		"Ssid", SsidValue(ssid));
	
	apDevice = wifi.Install(phy, mac, wifiApNode);


	// make 2nd ap-sta pair
	ssid = Ssid("ns3-80211ac2");

	mac.SetType("ns3::StaWifiMac",
		"Ssid", SsidValue(ssid));

	staDevice2 = wifi.Install(phy, mac, wifiStaNode2);

	mac.SetType("ns3::ApWifiMac",
		"Ssid", SsidValue(ssid));

	apDevice2 = wifi.Install(phy, mac, wifiApNode2);

	// channel bonding setting
	Ptr<RegularWifiMac> m_mac = DynamicCast<RegularWifiMac>(DynamicCast<WifiNetDevice>(apDevice.Get(0))->GetMac());

	Ptr<MacLow> m_low = m_mac->GetLow();
	m_low->EnableChannelBonding();
	m_low->SetChannelManager(phy, 36, 40, WIFI_PHY_STANDARD_80211ac);


	m_mac = DynamicCast<RegularWifiMac>(DynamicCast<WifiNetDevice>(staDevice.Get(0))->GetMac());

	m_low = m_mac->GetLow();

	m_low->EnableChannelBonding();
	m_low->SetChannelManager(phy, 36, 40, WIFI_PHY_STANDARD_80211ac);

	m_mac = DynamicCast<RegularWifiMac>(DynamicCast<WifiNetDevice>(apDevice2.Get(0))->GetMac());

	m_low = m_mac->GetLow();
	m_low->EnableChannelBonding();
	m_low->SetChannelManager(phy, 36, 40, WIFI_PHY_STANDARD_80211ac);


	m_mac = DynamicCast<RegularWifiMac>(DynamicCast<WifiNetDevice>(staDevice2.Get(0))->GetMac());

	m_low = m_mac->GetLow();

	m_low->EnableChannelBonding();
	m_low->SetChannelManager(phy, 36, 40, WIFI_PHY_STANDARD_80211ac);

	// mobility.
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

	positionAlloc->Add(Vector(0.0, 0.0, 0.0));
	positionAlloc->Add(Vector(distance, 0.0, 0.0));
	positionAlloc->Add(Vector(0.0, 1.0, 0.0));
	positionAlloc->Add(Vector(distance, 1.0, 0.0));
	mobility.SetPositionAllocator(positionAlloc);

	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

	mobility.Install(wifiApNode);
	mobility.Install(wifiStaNode);
	mobility.Install(wifiApNode2);
	mobility.Install(wifiStaNode2);
	//Ãß°¡

	/* Internet stack*/
	InternetStackHelper stack;
	stack.Install(wifiApNode);
	stack.Install(wifiStaNode);
	stack.Install(wifiApNode2);
	stack.Install(wifiStaNode2);

	Ipv4AddressHelper address;

	address.SetBase("192.168.1.0", "255.255.255.0");
	Ipv4InterfaceContainer staNodeInterface, staNodeInterface2;

	staNodeInterface = address.Assign(staDevice);
	address.Assign(apDevice);
	staNodeInterface2 = address.Assign(staDevice2);
	address.Assign(apDevice2);

	/* Setting applications */
	ApplicationContainer serverApp, serverApp2;

	//UDP flow
	UdpServerHelper myServer(9);
	serverApp = myServer.Install(wifiStaNode.Get(0));
	serverApp.Start(Seconds(0.0));
	serverApp.Stop(Seconds(simulationTime + 1));

	serverApp2 = myServer.Install(wifiStaNode2.Get(0));
	serverApp2.Start(Seconds(0.0));
	serverApp2.Stop(Seconds(simulationTime + 1));

	UdpClientHelper myClient(staNodeInterface.GetAddress(0), 9);
	myClient.SetAttribute("MaxPackets", UintegerValue(4294967295u));
	myClient.SetAttribute("Interval", TimeValue(Time("0.00001"))); //packets/s
	myClient.SetAttribute("PacketSize", UintegerValue(payloadSize));

	ApplicationContainer clientApp = myClient.Install(wifiApNode.Get(0));
	clientApp.Start(Seconds(1.0));
	clientApp.Stop(Seconds(simulationTime + 1));

	
	myClient.SetAttribute("RemoteAddress",AddressValue(staNodeInterface2.GetAddress(0)));

	ApplicationContainer clientApp2 = myClient.Install(wifiApNode2.Get(0));
	clientApp2.Start(Seconds(1.0));
	clientApp2.Stop(Seconds(simulationTime + 1));

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	Simulator::Stop(Seconds(simulationTime + 1));
	Simulator::Run();
	Simulator::Destroy();

	double throughput = 0;

	uint32_t totalPacketsThrough = DynamicCast<UdpServer>(serverApp.Get(0))->GetReceived();
	throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s


	std::cout << 40 << " MHz\t\t\t" << throughput << " Mbit/s" << std::endl;


	return 0;
}

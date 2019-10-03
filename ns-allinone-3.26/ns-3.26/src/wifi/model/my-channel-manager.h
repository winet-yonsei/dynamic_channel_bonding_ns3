#ifndef MY_CHANNEL_MANAGER_H
#define MY_CHANNEL_MANAGER_H

#include "wifi-mac-header.h"
#include "wifi-mode.h"
#include "wifi-phy.h"
#include "wifi-preamble.h"
#include "wifi-remote-station-manager.h"
#include "ctrl-headers.h"
#include "mgt-headers.h"
#include "block-ack-agreement.h"
#include "ns3/mac48-address.h"
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "qos-utils.h"
#include "block-ack-cache.h"
#include "wifi-tx-vector.h"
#include "mpdu-aggregator.h"
#include "msdu-aggregator.h"
#include "mac-low.h"
#include "ns3/wifi-helper.h"
#include "ampdu-tag.h"



namespace ns3 {
class MacLow;

typedef struct    // Struct to contain channel bonding tree
{
	uint16_t Secondary_Ch;	// Secondary channel
	uint32_t Width;
	uint16_t L_CHD, R_CHD;	// Children channels
	uint16_t Parent;	 // Parrent nodes
}ChannelInfo;

typedef struct  // Struct to contain errors occured same time (=duplicate packet)
{
	Time ErrorTime;
	std::vector< Ptr<Packet> > ErrorPacket;
	double rxSnr;
}error_packet_info;


class ChannelBondingManager : public Object
{
public:
	static TypeId GetTypeId (void);


	ChannelBondingManager();
	virtual ~ChannelBondingManager();
	uint16_t GetPrimaryCh();           // Return primary channel number
	uint32_t GetMaxWidth();            // Return max channel width
	uint32_t GetRequestWidth();        // Return current channel width
	
	void SetChannelOption(uint16_t Primary_Ch,uint32_t Max_Width);  // Set primary channel & max channel width
	void ChangeMaxWidth(uint32_t Max_Width);                        // Change max channel width
	void MakePhys(const WifiPhyHelper &phy, Ptr<WifiPhy> primary, uint16_t ch_num, uint32_t channel_width, enum WifiPhyStandard standard);   // Create subchannels phy

	void CheckChannelBeforeSend(void);                 // Adjust channel width 

	void ResetPhys();                                 // Remove all subchannels

	void SendPacket (Ptr<const Packet> packet, WifiTxVector txVector, enum WifiPreamble preamble, enum mpduType mpdutype);  // Send duplicate packets through subchannels
	void SendPacket(Ptr<const Packet> packet, WifiTxVector txVector, enum WifiPreamble preamble);

	static std::map<uint16_t, ChannelInfo> ChannelMapping();             // Make bonded channels map

	void ClearReceiveRecord();                   // Clear last received packet record

	Ptr<Packet> ConvertPacket(Ptr<const Packet> packet);   // Duplicate packets


	void SetPhysCallback();
	void ManageReceived (Ptr<Packet> packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);  // Integrate received packets on all sub channels
	void SetMyMac(Ptr<MacLow> mac);
	void NeedRtsCts(bool need);                   // Enable RTS/CTS
	
	std::map<uint16_t, Ptr<WifiPhy>> GetPhys();


private:
	Time last_receive_or_error_time;  // Last time occuring error or receiving packet
	uint16_t RECount; // Count of receiving packet and error
	uint16_t RECountLimit;  // Number of events required (0: unknown)
	double MinSnr; // The minimum SNR among the packets
	bool isErr, ErrReport; // Error flag, report error flag

	Ptr<Packet> last_packet; // Latest errored/received packet
	uint32_t max_width;  // Maximum channel width parameter

	uint32_t request_width;  // Channel width parameter of sending packet 
	uint16_t request_ch;  // Channel number parameter of sending packet 


	uint16_t primary_ch;   // Primary channel number
	std::vector<uint16_t> ch_numbers;  // Channel number of all channels
	error_packet_info error_packets;   // Recevied error packets

	Ptr<MacLow> m_mac;   // Pointer of MacLow
	std::map< uint16_t, Ptr<Packet> > last_received_packet;  // Last receive packets
	std::map< uint16_t, Ptr<Packet> > packet_pieces;   // Modified packet for send
	
	std::map<uint16_t, Ptr<WifiPhy> > m_phys;  // Phys for channels

	std::map<int,bool> received_channel;  // Receive packet flag for individual channels

	bool need_rts_cts;  // Flag of using rts/cts

	uint16_t CheckChBonding(uint16_t primary);	// Find widest usable channel (only consider idle condition)

	bool CheckAllSubChannelIdle(uint16_t ch_num);   // Check that all subchannels that make up the channel bonding are idle

	uint16_t GetUsableBondingChannel(uint16_t primary);  // Get suitable channel bonding considering RTS/CTS event

	bool CheckAllSubChannelReceived(uint16_t ch_num);  // Ensure that all subchannels of the channel bonding received an RTS-CTS packet

	uint16_t GetChannelWithWidth(uint32_t width);    // Width -> merged channel number

	void CleanPacketPieces();                                // Clear storage of duplicated packets for sending
	std::vector<uint16_t> FindSubChannels(uint16_t ch_num);  // Return all subchannels that make up the merged channel.

	uint8_t GetNumberOfReceive();                   // The number of duplicate packets received

	void SetUpChannelNumbers();                           // Assign channel number to all channels

	int CheckError(Ptr<const Packet> Packet);            // Check error is occured 

	// when phy receive packet, this function operate
	void Receive1Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive2Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive3Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive4Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive5Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive6Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive7Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive8Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void ReceiveSubChannel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble, uint16_t ch_num); // Packets received on one subchannel

	void ReceiveOk (Ptr<Packet> packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble, bool ampduSubframe);

	// When phy receive error, this function operate
	void Error1Channel(Ptr<Packet> packet, double rxSnr);
	void Error2Channel(Ptr<Packet> packet, double rxSnr);
	void Error3Channel(Ptr<Packet> packet, double rxSnr);
	void Error4Channel(Ptr<Packet> packet, double rxSnr);
	void Error5Channel(Ptr<Packet> packet, double rxSnr);
	void Error6Channel(Ptr<Packet> packet, double rxSnr);
	void Error7Channel(Ptr<Packet> packet, double rxSnr);
	void Error8Channel(Ptr<Packet> packet, double rxSnr);

	void Error (Ptr<Packet> packet, double rxSnr, uint16_t ch_num);  //// Integrate errors on sub channels

	bool CheckItFirst(Ptr<Packet> packet);   // Checks if the same packet was previously received

	const std::map < uint16_t, ChannelInfo > ch_map;	// Channel map for merged channel
};

}
#endif

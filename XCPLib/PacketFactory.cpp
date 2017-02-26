#include "PacketFactory.h"
#include "ConnectPositivePacket.h"
#include "GetStatusPacket.h"
#include "SynchPacket.h"
#include "DisconnectPacket.h"
#include <iostream>


IXCPPacket * PacketFactory::CreateResponsePacket(const std::vector<uint8_t>& Data, uint8_t HeaderSize, CommandPacket * LastSentCommand)
{
	uint8_t LastCommandPID = LastSentCommand->GetPid();
	switch (LastCommandPID)
	{
	case CTOMasterToSlaveCommands::CONNECT:
		return ConnectPositivePacket::Deserialize(Data, HeaderSize);
		break;
	case CTOMasterToSlaveCommands::DISCONNECT:
		return new ResponsePacket();
		break;
	case CTOMasterToSlaveCommands::GET_STATUS:
		return GetStatusResponsePacket::Deserialize(Data, HeaderSize);
		break;
	default:
		std::cout << "Unhandled response format\n";
		return nullptr;
		break;
	}
}

IXCPPacket * PacketFactory::CreateErrorPacket(const std::vector<uint8_t>& data, uint8_t header_size, CommandPacket * LastSentCommand)
{
	uint8_t ErrorCode = data[header_size + 1];
	uint8_t LastCommandPID = LastSentCommand->GetPid();
	switch (ErrorCode)
	{
	case ErrorCodes::ERR_CMD_SYNCH:
		return new SynchResponsePacket();
		break;
	default:
		std::cout << "Deserialization error: Unhandled errorcode\n";
		return nullptr;
		break;
	}
}

PacketFactory::PacketFactory()
{
}


PacketFactory::~PacketFactory()
{
}

IXCPPacket * PacketFactory::CreateConnectPacket(ConnectMode mode)
{
	return new ConnectPacket(mode);
}

IXCPPacket * PacketFactory::CreateDisconnectPacket()
{
	return new DisconnectPacket();
}

IXCPPacket * PacketFactory::CreateGetStatusPacket()
{
	return new GetStatusPacket();
}

IXCPPacket * PacketFactory::CreateSynchPacket()
{
	return new SynchPacket();
}

IXCPPacket * PacketFactory::DeserializeIncomingFromSlave(const std::vector<uint8_t>& Data, uint8_t HeaderSize, CommandPacket* LastSentCommand)
{
	uint8_t PID = Data[HeaderSize];
	switch (PID)
	{
	case CTOSlaveToMasterPacketTypes::RES:
		return CreateResponsePacket(Data, HeaderSize, LastSentCommand);
		break;
	case CTOSlaveToMasterPacketTypes::EV:
		break;
	case CTOSlaveToMasterPacketTypes::ERR:
		return CreateErrorPacket(Data, HeaderSize, LastSentCommand);
		break;
	case CTOSlaveToMasterPacketTypes::SERV:
		break;
	default:
		break;
	}
	return nullptr;
}

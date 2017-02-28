#include "XCPMaster.h"
#include "TCPMessageFactory.h"
#include "IncomingMessageHandler.h"
#include <vld.h>

const XCPMaster::SlaveProperties& XCPMaster::GetSlaveProperties() const
{
	return m_SlaveProperties;
}

void XCPMaster::SetSlaveProperties(const SlaveProperties& properties)
{
	m_SlaveProperties = properties;
}

XCPMaster::XCPMaster(TransportLayer transportlayer)
{
	switch (transportlayer)
	{
	case TransportLayer::ETHERNET:
		m_MessageFactory = new TCPMessageFactory();
		break;
	//case TransportLayer::CAN:
	//	break;
	default:
		m_MessageFactory = nullptr;
		break;
	}

	m_PacketFactory = new PacketFactory(*this);
	m_MessageHandler = new IncomingMessageHandler(*this);
}

XCPMaster::~XCPMaster()
{
	delete m_MessageFactory;
	delete m_PacketFactory;
	delete m_MessageHandler;
}

std::unique_ptr<IXCPMessage> XCPMaster::CreateConnectMessage(ConnectMode mode)
{
	if (!m_MessageFactory)
	{
		return nullptr;
	}

	return std::unique_ptr<IXCPMessage>(m_MessageFactory->CreateMessage(m_PacketFactory->CreateConnectPacket(mode)));
}

std::unique_ptr<IXCPMessage> XCPMaster::CreateDisconnectMessage()
{
	if (!m_MessageFactory)
	{
		return nullptr;
	}

	return std::unique_ptr<IXCPMessage>(m_MessageFactory->CreateMessage(m_PacketFactory->CreateDisconnectPacket()));
}

std::unique_ptr<IXCPMessage> XCPMaster::CreateGetStatusMessage()
{
	if (!m_MessageFactory)
	{
		return nullptr;
	}

	return std::unique_ptr<IXCPMessage>(m_MessageFactory->CreateMessage(m_PacketFactory->CreateGetStatusPacket()));
}

std::unique_ptr<IXCPMessage> XCPMaster::CreateSynchMessage()
{
	if (!m_MessageFactory)
	{
		return nullptr;
	}

	return std::unique_ptr<IXCPMessage>(m_MessageFactory->CreateMessage(m_PacketFactory->CreateSynchPacket()));
}

XCP_API std::unique_ptr<IXCPMessage> XCPMaster::CreateSetMTAMessage(uint32_t address, uint8_t extension)
{
	if (!m_MessageFactory)
	{
		return nullptr;
	}

	return std::unique_ptr<IXCPMessage>(m_MessageFactory->CreateMessage(m_PacketFactory->CreateSetMTAPacket(address,extension,m_SlaveProperties.ByteOrder==0)));
}

std::unique_ptr<IXCPMessage> XCPMaster::DeserializeMessage(std::vector<uint8_t>& data)
{
	IXCPPacket* Packet = m_PacketFactory->DeserializeIncomingFromSlave(data, m_MessageFactory->GetHeaderSize(), m_SentCommandQueue.front());
	m_SentCommandQueue.pop();
	if (Packet)
	{
		IXCPMessage* MessageFrame = m_MessageFactory->CreateMessage(Packet);
		MessageFrame->SetPacket(Packet);
		if (m_MessageHandler)
		{
			Packet->Dispatch(*m_MessageHandler);
		}
		return std::unique_ptr<IXCPMessage>(MessageFrame);
	}
	std::cout << "couldnt deserialise the message\n";
	return nullptr;
}

XCP_API std::unique_ptr<IXCPMessage> XCPMaster::CreateUploadMessage(uint8_t NumberOfElements)
{
	if (!m_MessageFactory)
	{
		return nullptr;
	}

	return std::unique_ptr<IXCPMessage>(m_MessageFactory->CreateMessage(m_PacketFactory->CreateUploadPacket(NumberOfElements)));
}

void XCPMaster::AddSentMessage(IXCPMessage * Packet)
{
	if (CommandPacket* ToAdd = dynamic_cast<CommandPacket*>(Packet->GetPacket()))
	{
		m_SentCommandQueue.push(ToAdd);
	}
	else
	{
		std::cout << "XCPMaster::AddSentMessage: This is not a CMD packet.";
	}
}

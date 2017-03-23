#include "DTO.h"
#include "DAQPackets.h"
using ModeFieldBits = SetDaqListModePacket::ModeFieldBits;
using IdFieldBits = GetDaqProcessorInfoResponse::DaqKeyByteBits;

DTO::DTO(const std::vector<uint8_t>& Data, uint8_t HeadSize, uint8_t TailSize, uint8_t Mode, uint8_t TimestampSize, bool TimestampFixed, uint8_t IdentificationFieldType, bool FirstODT) : IXCPPacket()
{
	IsTimestamped = false;
	m_Timestamp = 0;
	IsCTRed = false;
	m_CTR = 0;
	m_Fill = 0;
	m_DAQ = 0;

	uint32_t ptr = HeadSize;
	if (Mode&ModeFieldBits::PID_OFF)
	{
		throw 0; //TODO: support PID_OFF
	}
	else
	{
		m_PID = Data[ptr]; //between 0x00-0xFB
		ptr++;
		if (IdentificationFieldType == GetDaqProcessorInfoResponse::IdentificationFieldType::ABSOLUTE_ODT_NUMBER)
		{
			if (FirstODT && (ModeFieldBits::DTO_CTR&Mode))
			{
				m_CTR = Data[ptr];
				ptr++;
			}
		}
		else if (IdentificationFieldType == GetDaqProcessorInfoResponse::IdentificationFieldType::RELATIVE_ODT_ABSOLUTE_DAQ_BYTE)
		{
			m_DAQ = Data[ptr];
			ptr++;
			if (FirstODT && (ModeFieldBits::DTO_CTR&Mode))
			{
				m_CTR = Data[ptr];
				ptr++;
			}
		}
		else if (IdentificationFieldType == GetDaqProcessorInfoResponse::IdentificationFieldType::RELATIVE_ODT_ABSOLUTE_DAQ_WORD)
		{
			uint16_t t1 = Data[ptr];
			uint16_t t2 = Data[ptr + 1];
			m_DAQ = t1 & 0xFF;
			m_DAQ |= (t2 << 8) & 0xFF00;
			if (FirstODT && (ModeFieldBits::DTO_CTR&Mode))
			{
				m_CTR = Data[ptr];
				ptr += 3;
			}
			else
			{
				ptr += 2;
			}
		}
		else if (IdentificationFieldType == GetDaqProcessorInfoResponse::IdentificationFieldType::RELATIVE_ODT_ABSOLUTE_DAQ_WORD_ALIGNED)
		{
			m_Fill = Data[ptr];
			uint16_t t1 = Data[ptr + 1];
			uint16_t t2 = Data[ptr + 2];
			m_DAQ = t1 & 0xFF;
			m_DAQ |= (t2 << 8) & 0xFF00;
			ptr += 3;
		}
	}

	if ((Mode&ModeFieldBits::TIMESTAMP || TimestampFixed) && FirstODT)
	{
		IsTimestamped = true;
		if (TimestampSize == 1)
		{
			m_Timestamp = Data[ptr];
			ptr++;
		}
		else if (TimestampSize == 2)
		{
			uint32_t t1, t2;
			t1 = Data[ptr];
			t2 = Data[ptr + 1];
			ptr += 2;
			m_Timestamp = t1 & 0xFF;
			m_Timestamp |= ((t2 << 8) & 0xFF00);
		}
		else if (TimestampSize == 4)
		{
			uint32_t t1, t2, t3, t4;
			t1 = Data[ptr];
			t2 = Data[ptr + 1];
			t3 = Data[ptr + 2];
			t4 = Data[ptr + 3];
			ptr += 4;
			m_Timestamp = t1 & 0xFF;
			m_Timestamp |= ((t2 << 8) & 0x0000FF00);
			m_Timestamp |= ((t3 << 16) & 0x00FF0000);
			m_Timestamp |= ((t4 << 24) & 0xFF000000);
		}
	}

	m_DataLength = Data.size() - TailSize - ptr;
	m_Data = new uint8_t[m_DataLength];
	for (uint32_t i = 0; i < m_DataLength; i++)
	{
		m_Data[i] = Data[i + ptr];
	}
	m_PacketSize = m_DataLength + ptr - HeadSize;
}

DTO::~DTO()
{
	delete[] m_Data;
}

uint16_t DTO::GetDAQField()
{
	return m_DAQ;
}

uint8_t DTO::GetCTRField()
{
	return m_CTR;
}

uint8_t DTO::GetFillField()
{
	return m_Fill;
}

uint32_t DTO::GetTimestamp()
{
	return m_Timestamp;
}

uint8_t DTO::GetByteElement(uint32_t i)
{
	if (i < m_DataLength)
	{
		return m_Data[i];
	}
	return 0;
}

bool DTO::GetIsTimestamped()
{
	return IsTimestamped;
}

bool DTO::GetIsCTRed()
{
	return IsCTRed;
}

void DTO::Dispatch(IIncomingMessageHandler & Handler)
{
	Handler.Handle(*this);
}

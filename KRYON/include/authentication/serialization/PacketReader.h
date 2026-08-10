#ifndef KRYON_PACKET_READER_H
#define KRYON_PACKET_READER_H

#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace kryon
{

class PacketReader
{
public:

    PacketReader(
        const std::vector<uint8_t>& buffer)
        :
        m_buffer(buffer),
        m_position(0)
    {
    }

    template<typename T>
    T Read()
    {
        if (m_position + sizeof(T) > m_buffer.size())
        {
            throw std::runtime_error(
                "PacketReader: buffer underflow.");
        }

        T value;

        std::memcpy(
            &value,
            m_buffer.data() + m_position,
            sizeof(T));

        m_position += sizeof(T);

        return value;
    }

    std::string ReadString()
    {
        uint32_t size = Read<uint32_t>();

        if (m_position + size > m_buffer.size())
        {
            throw std::runtime_error(
                "PacketReader: invalid string size.");
        }

        std::string value(
            reinterpret_cast<const char*>(
                m_buffer.data() + m_position),
            size);

        m_position += size;

        return value;
    }

    std::vector<uint8_t> ReadVector()
    {
        uint32_t size = Read<uint32_t>();

        if (m_position + size > m_buffer.size())
        {
            throw std::runtime_error(
                "PacketReader: invalid vector size.");
        }

        std::vector<uint8_t> value(
            m_buffer.begin() + m_position,
            m_buffer.begin() + m_position + size);

        m_position += size;

        return value;
    }

    bool HasRemaining() const
    {
        return m_position < m_buffer.size();
    }

private:

    const std::vector<uint8_t>& m_buffer;

    size_t m_position;
};

}

#endif

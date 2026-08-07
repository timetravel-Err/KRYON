#ifndef KRYON_PACKET_WRITER_H
#define KRYON_PACKET_WRITER_H

#include <vector>
#include <string>
#include <cstring>
#include <cstdint>

namespace kryon
{

class PacketWriter
{
public:

    template<typename T>
    void Write(const T& value)
    {
        const uint8_t* ptr =
            reinterpret_cast<const uint8_t*>(&value);

        m_buffer.insert(
            m_buffer.end(),
            ptr,
            ptr + sizeof(T));
    }

    void WriteString(const std::string& value)
    {
        uint32_t size = value.size();

        Write(size);

        m_buffer.insert(
            m_buffer.end(),
            value.begin(),
            value.end());
    }

    void WriteVector(const std::vector<uint8_t>& value)
    {
        uint32_t size = value.size();

        Write(size);

        m_buffer.insert(
            m_buffer.end(),
            value.begin(),
            value.end());
    }

    const std::vector<uint8_t>& GetBuffer() const
    {
        return m_buffer;
    }

private:

    std::vector<uint8_t> m_buffer;

};

}

#endif

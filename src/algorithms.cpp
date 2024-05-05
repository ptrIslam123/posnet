#include "include/utils/algorithms.h"

namespace posnet::utils {

std::uint16_t CalcChecksum(std::span<const std::uint8_t> packet)
{
    uint32_t sum = 0;  // Используем uint32_t для хранения суммы, чтобы избежать переполнения

    // Суммируем все 16-битные слова
    for (auto i = 0; i + 1 < packet.size(); i += 2) {
        sum += (packet[i] << 8) | packet[i + 1];
    }

    // Если есть нечетный байт, добавляем его в сумму
    if (packet.size() % 2 == 1) {
        sum += packet[packet.size() - 1] << 8;
    }

    // Складываем старшие биты (если есть перенос) с младшими
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // Дополнение до единицы
    sum = ~sum;

    // Возвращаем 16-битный результат
    return static_cast<uint16_t>(sum & 0xFFFF);
}

std::uint16_t CalcChecksum(const std::span<std::uint8_t> packet) 
{
    std::size_t i;
    std::uint64_t sum = 0;
    auto buffer = packet.data();
    const auto size = packet.size();
    for (i = 0; i < size; i += 2) {
        sum += *(uint16_t *)buffer;
        buffer += 2;
    }

    if (size - i > 0) {
        sum += *(uint8_t *)buffer;
    }

    while ((sum >> 16) != 0) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return (uint16_t)~sum;
}

void HostBufferViewToNetwork(std::span<std::int8_t> buffer)
{
    //! TODO:
}

void HostBufferViewToNetwork(std::span<std::uint8_t> buffer)
{
    //! TODO:
}

} //! namespace posnet::utils
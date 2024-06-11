#ifndef VS_DNS_BUILDER_H
#define VS_DNS_BUILDER_H

#include "include/base_frame.h"
#include "include/frame-viewers/dns_viewer.h"
#include "include/utils/io/sync/streambuffer/istreambuffer.h"

#include <string>
#include <string_view>
#include <ostream>
#include <array>
#include <cstring>

namespace posnet {

class DnsBuilder final : public BaseFrame {
public:
    static constexpr auto DEFAULT_PORT = DnsViewer::DEFAULT_UDP_PORT;
    static constexpr auto TURN_ON_FLAG = DnsViewer::TURN_ON_FLAG;
    static constexpr auto TURN_OFF_FLAG =DnsViewer::TURN_OFF_FLAG;

    using HeaderStructType = DnsViewer::HeaderStructType;
    using QuestionStructType = DnsViewer::QuestionStructType;
    using AnswerStructType = DnsViewer::AnswerStructType;
    using AuthorityStructType = DnsViewer::AuthorityStructType;

    using Type = DnsViewer::Type;
    using RecordCodeType = DnsViewer::RecordCodeType;
    using QueryType = DnsViewer::QueryType;
    using QueryClass = DnsViewer::QueryClass;
    using OpcodeType = DnsViewer::OpcodeType;
    using AuthorityType = DnsViewer::AuthorityType;
    using AuthorityClass = DnsViewer::AuthorityClass;

    template<typename Allocator>
    using IStreamBuffer = posnet::utils::io::IStreamBuffer<Allocator>;

    class HeaderBuilder final {
    public:
        explicit HeaderBuilder();
        HeaderBuilder& setId(unsigned short id) &;
        HeaderBuilder& setType(Type query) &;
        HeaderBuilder& setOpcode(OpcodeType opcode) &;
        HeaderBuilder& setFlagAA(bool status) &;
        HeaderBuilder& setFlagTC(bool status) &;
        HeaderBuilder& setFlagRD(bool status) &;
        HeaderBuilder& setFlagRA(bool status) &;
        HeaderBuilder& setFlagRCODE(RecordCodeType rcode) &;
        HeaderBuilder& setQueryCount(unsigned short count) &;
        HeaderBuilder& setAnswerCount(unsigned short count) &;
        HeaderBuilder& setAuthorityCount(unsigned short count) &;
        HeaderBuilder& setAdditionalRecordCount(unsigned short count) &;

        template<typename Allocator>
        IStreamBuffer<Allocator>& operator<<(IStreamBuffer<Allocator>& istreamBuffer);

    private:
        friend DnsBuilder;
        HeaderStructType m_header;
    };

    class QuestionBuilder final {
    public:
        explicit QuestionBuilder();
        QuestionBuilder& setName(std::span<std::uint8_t> name) &;
        QuestionBuilder& setType(QueryType recordType) &;
        QuestionBuilder& setClass(QueryClass queryClass) &;

        template<typename Allocator>
        IStreamBuffer<Allocator>& operator<<(IStreamBuffer<Allocator>& istreamBuffer);

    private:
        friend DnsBuilder;
        std::vector<std::uint8_t> m_questionName;
        QuestionStructType m_question;
    };

    class AuthorityBuilder final {
    public:
        explicit AuthorityBuilder();
        AuthorityBuilder& setName(std::span<std::uint8_t> name) &;
        AuthorityBuilder& setType(AuthorityType authorityType) &;
        AuthorityBuilder& setClass(AuthorityClass authorityClass) &;
        AuthorityBuilder& setRecordDataLength(unsigned short length) &;
        AuthorityBuilder& setRecordData(std::span<std::uint8_t> recordData) &;

        template<typename Allocator>
        IStreamBuffer<Allocator>& operator<<(IStreamBuffer<Allocator>& istreamBuffer);

    private:
        friend DnsBuilder;
        std::vector<std::uint8_t> m_authorityName;
        AuthorityStructType m_authority;
        std::vector<std::uint8_t> m_authorityRecordData;
    };

    explicit DnsBuilder(
            const HeaderBuilder& header,
            std::span<QuestionBuilder> questions,
            std::span<AuthorityBuilder> authorities = std::span<AuthorityBuilder>{}
    );

    std::ostream& operator<<(std::ostream& os);
    template<typename Allocator>
    IStreamBuffer<Allocator>& operator<<(IStreamBuffer<Allocator>& istreamBuffer);

    static std::string StrToDNSDomainFormat(std::string_view domain);

private:
    HeaderBuilder m_header;
    std::vector<QuestionBuilder> m_questions;
    std::vector<AuthorityBuilder> m_authorities;
};

template<typename Allocator>
DnsBuilder::IStreamBuffer<Allocator>& DnsBuilder::HeaderBuilder::operator<<(IStreamBuffer<Allocator>& istreamBuffer)
{
    istreamBuffer.writeBytes(
                std::span<std::uint8_t>{ reinterpret_cast<std::uint8_t*>(&m_header), sizeof(m_header) }
    );
    return istreamBuffer;
}

template<typename Allocator>
DnsBuilder::IStreamBuffer<Allocator>& DnsBuilder::QuestionBuilder::operator<<(IStreamBuffer<Allocator>& istreamBuffer)
{
    istreamBuffer.writeBytes(std::span<std::uint8_t>{ m_questionName });
    istreamBuffer.writeBytes(
                std::span<std::uint8_t>{ reinterpret_cast<std::uint8_t*>(&m_question), sizeof(m_question) }
    );
    return istreamBuffer;
}

template<typename Allocator>
DnsBuilder::IStreamBuffer<Allocator>& DnsBuilder::AuthorityBuilder::operator<<(IStreamBuffer<Allocator>& istreamBuffer)
{
    istreamBuffer.writeBytes(std::span<std::uint8_t>{ m_authorityName });
    istreamBuffer.writeBytes(
                std::span<std::uint8_t>{ reinterpret_cast<std::uint8_t*>(&m_authority), sizeof(m_authority) }
    );
    istreamBuffer.writeBytes(std::span<std::uint8_t>{ m_authorityRecordData });
    return istreamBuffer;
}

template<typename Allocator>
DnsBuilder::IStreamBuffer<Allocator>& operator<<(DnsBuilder::IStreamBuffer<Allocator>& istreamBuffer, DnsBuilder& dnsBuilder)
{
    return dnsBuilder.operator<<(istreamBuffer);
}

template<typename Allocator>
DnsBuilder::IStreamBuffer<Allocator>& DnsBuilder::operator<<(IStreamBuffer<Allocator>& istreamBuffer)
{
    using ConstBufferViewType = typename IStreamBuffer<Allocator>::ConstBufferViewType;
    m_header.operator<<(istreamBuffer);

    for (auto& question : m_questions) {
        question.operator<<(istreamBuffer);
    }

    for (auto& authority : m_authorities) {
        authority.operator<<(istreamBuffer);
    }
    return istreamBuffer;
}

} //! namespace posnet

#endif //! VS_DNS_BUILDER_H

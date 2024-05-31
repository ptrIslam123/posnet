#include "include/frame-builder/dns_builder.h"

#include <netinet/in.h>

namespace posnet {

DnsBuilder::DnsBuilder(
        const HeaderBuilder& header,
        const std::span<QuestionBuilder> questions,
        const std::span<AuthorityBuilder> authorities
):
BaseFrame(nullptr, 0),
m_header(header),
m_questions(),
m_authorities()
{
    m_questions.insert(m_questions.end(), questions.begin(), questions.end());
    m_authorities.insert(m_authorities.end(), authorities.begin(), authorities.end());
}

DnsBuilder::HeaderBuilder::HeaderBuilder()
{
    std::memset(&m_header, 0, sizeof(m_header));
}

DnsBuilder::HeaderBuilder& DnsBuilder::HeaderBuilder::setId(const unsigned short id) &
{
    m_header.id = htons(id);
    return *this;
}

DnsBuilder::HeaderBuilder& DnsBuilder::HeaderBuilder::setType(const Type query) &
{
    m_header.qr = static_cast<decltype(m_header.qr)>(query);
    return *this;
}

DnsBuilder::HeaderBuilder& DnsBuilder::HeaderBuilder::setOpcode(const OpcodeType opcode) &
{
    m_header.opcode = static_cast<decltype(m_header.opcode)>(opcode);
    return *this;
}

DnsBuilder::HeaderBuilder& DnsBuilder::HeaderBuilder::setFlagAA(const bool status) &
{
    m_header.aa = static_cast<decltype(m_header.aa)>(status);
    return *this;
}

DnsBuilder::HeaderBuilder& DnsBuilder::HeaderBuilder::setFlagTC(const bool status) &
{
    m_header.tc = static_cast<decltype(m_header.tc)>(status);
    return *this;
}

DnsBuilder::HeaderBuilder& DnsBuilder::HeaderBuilder::setFlagRD(const bool status) &
{
    m_header.rd = static_cast<decltype(m_header.rd)>(status);
    return *this;
}

DnsBuilder::HeaderBuilder& DnsBuilder::HeaderBuilder::setFlagRA(const bool status) &
{
    m_header.ra = static_cast<decltype(m_header.ra)>(status);
    return *this;
}

DnsBuilder::HeaderBuilder& DnsBuilder::HeaderBuilder::setFlagRCODE(const RecordCodeType rcode) &
{
    m_header.rcode = static_cast<decltype(m_header.rcode)>(rcode);
    return *this;
}

DnsBuilder::HeaderBuilder& DnsBuilder::HeaderBuilder::setQueryCount(const unsigned short count) &
{
    m_header.qdcount = htons(count);
    return *this;
}

DnsBuilder::HeaderBuilder& DnsBuilder::HeaderBuilder::setAnswerCount(const unsigned short count) &
{
    m_header.ancount = htons(count);
    return *this;
}

DnsBuilder::HeaderBuilder& DnsBuilder::HeaderBuilder::setAuthorityCount(const unsigned short count) &
{
    m_header.nscount = htons(count);
    return *this;
}

DnsBuilder::HeaderBuilder& DnsBuilder::HeaderBuilder::setAdditionalRecordCount(const unsigned short count) &
{
    m_header.arcount = htons(count);
    return *this;
}

DnsBuilder::QuestionBuilder::QuestionBuilder()
{
    std::memset(&m_question, 0, sizeof(m_question));
}

DnsBuilder::QuestionBuilder& DnsBuilder::QuestionBuilder::setName(const std::span<std::uint8_t> name) &
{
    m_questionName.insert(m_questionName.end(), name.begin(), name.end());
    return *this;
}

DnsBuilder::QuestionBuilder& DnsBuilder::QuestionBuilder::setType(QueryType recordType) &
{
    m_question.qtype = htons(static_cast<decltype(m_question.qtype)>(recordType));
    return *this;
}

DnsBuilder::QuestionBuilder& DnsBuilder::QuestionBuilder::setClass(QueryClass queryClass) &
{
    m_question.qclass = htons(static_cast<decltype(m_question.qclass)>(queryClass));
    return *this;
}

DnsBuilder::AuthorityBuilder::AuthorityBuilder()
{
    std::memset(&m_authority, 0, sizeof(m_authority));
}

DnsBuilder::AuthorityBuilder& DnsBuilder::AuthorityBuilder::setName(const std::span<std::uint8_t> name) &
{
    m_authorityName.insert(m_authorityName.end(), name.begin(), name.end());
    return *this;
}

DnsBuilder::AuthorityBuilder& DnsBuilder::AuthorityBuilder::setType(const AuthorityType authorityType) &
{
    m_authority.atype = ntohs(static_cast<decltype(m_authority.atype)>(authorityType));
    return *this;
}

DnsBuilder::AuthorityBuilder& DnsBuilder::AuthorityBuilder::setClass(const AuthorityClass authorityClass) &
{
    m_authority.aclass = ntohs(static_cast<decltype(m_authority.aclass)>(authorityClass));
    return *this;
}

DnsBuilder::AuthorityBuilder& DnsBuilder::AuthorityBuilder::setRecordDataLength(const unsigned short length) &
{
    m_authority.rdlength = ntohs(length);
    return *this;
}

DnsBuilder::AuthorityBuilder& DnsBuilder::AuthorityBuilder::setRecordData(std::span<std::uint8_t> recordData) &
{
    m_authorityRecordData.insert(m_authorityRecordData.end(), recordData.begin(), recordData.end());
    return *this;
}

std::string DnsBuilder::StrToDNSDomainFormat(const std::string_view domain) {
    std::string_view label = domain;
    std::string storage;
    while (!label.empty()) {
        const auto nextDot = label.find('.');
        const auto labelLength = (nextDot != std::string::npos) ? nextDot : label.size();
        storage.push_back(labelLength);
        std::copy(label.begin(), label.begin() + labelLength, std::back_inserter(storage));
        label = (nextDot != std::string::npos) ? label.substr(nextDot + 1) : std::string_view{};
    }
    storage.push_back(0); // Null label to indicate the end of the name
    return storage;
}

std::ostream& DnsBuilder::operator<<(std::ostream& os)
{
    return os;
}

} //! namespace posnet

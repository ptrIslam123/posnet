#include "include/frame-viewers/dns_viewer.h"
#include "include/frame-builder/dns_builder.h"
#include "include/utils/sock_addr_convertor.h"
#include "include/utils/assert.h"
#include "include/utils/algorithms.h"
#include <iostream>
#include <arpa/inet.h>

namespace {

std::uint16_t ExtractOffsetFromCompressedPointer(const uint8_t* buffer) {
    const auto index = 0;
    // Check if the next two bytes are a compressed pointer
    if (buffer[index] != 0xC0) {
        return 0;
    }

    // Extract the offset from the next two bytes
    uint16_t offset = (buffer[index + 1] << 8) | buffer[index + 2];
    offset = ntohs(offset);
    return offset;
}

void SerializeToStream(std::ostream& os, const posnet::DnsViewer::HeaderViewer& headerViewer)
{
    using DnsViewer = posnet::DnsViewer;
     os << "DNS::Header-Section = {" << "\n";
    os << "\t" << "id=" << headerViewer.getId() << "\n";
    os << "\t" << "(rd)=" << headerViewer.getFlagRA() << "\n";
    os << "\t" << "(tc)=" << headerViewer.getFlagTC() << "\n";
    os << "\t" << "(aa)=" << headerViewer.getFlagAA() << "\n";
    os << "\t" << "opcode=" << DnsViewer::OpcodeTypeToStr(headerViewer.getOpcode()) << "\n";
    os << "\t" << "request type(qr)=" << DnsViewer::TypeToStr(headerViewer.getType()) << "\n";
    os << "\t" << "(ra)=" << headerViewer.getFlagRA() << "\n";
    os << "\t" << "query count(qdcount)=" << headerViewer.getQueryCount() << "\n";
    os << "\t" << "answer count(ancount)=" << headerViewer.getAnswerCount() << "\n";
    os << "\t" << "authority count(nscount)=" << headerViewer.getAuthorityCount() << "\n";
    os << "\t" << "record count(arcount)=" << headerViewer.getAnswerCount() << "\n";
    os << "}";
}

void SerializeToStream(std::ostream& os, const posnet::DnsViewer::QuestionViewer& questionViewer)
{
    using DnsViewer = posnet::DnsViewer;
    os << "DNS::Question-Section = {" << "\n";
    os << "\t" << "question-domain=" << questionViewer.getQueryDomain() << "\n";
    os << "\t" << "question-type(qtype)=" << DnsViewer::QueryTypeToStr(questionViewer.getQueryType()) << "\n";
    os << "\t" << "question-class(qclass)=" << DnsViewer::QueryClassToStr(questionViewer.getQueryClass()) << "\n";
    os << "}";
}

void SerializeToStream(std::ostream& os, const posnet::DnsViewer::AnswerViewer& answerViewer)
{
    using DnsViewer = posnet::DnsViewer;
    using Type = DnsViewer::AnswerType;
    os << "DNS::Answer-Section = {" << "\n";
    os << "\t" << "domain-name=" << answerViewer.getDomainName() << "\n";
    os << "\t" << "type=" << DnsViewer::AnswerTypeToStr(answerViewer.getType()) << "\n";
    os << "\t" << "class=" << DnsViewer::AnswerClassToStr(answerViewer.getClass()) << "\n";
    os << "\t" << "ttl=" << answerViewer.getTTL() << "\n";
    os << "\t" << "record-data-length=" << answerViewer.getRecordDataLength() << "\n";
    os << "\t" << "record-data";
    switch (answerViewer.getType()) {
        case Type::HostAddrIpV4: {
            os << "(ipV4-addr)=" << *answerViewer.castRecordDataToIpV4Addr() << "\n";
            break;
        }
        case Type::HostAddrIpV6: {
            os << "(ipV6-addr)=" << *answerViewer.castRecordDataToIpV6Addr() << "\n";
            break;
        }
        case Type::Text: {
            os << "(text)=" << *answerViewer.castRecordDataToText() << "\n";
            break;
        }
        default: {
            os << "(Unknown)" << "\n";
            break;
        }
    }
    os << "}";
}

void SerializeToStream(std::ostream& os, const posnet::DnsViewer& dnsViewer)
{
    using DnsViewer = posnet::DnsViewer;
    SerializeToStream(os, dnsViewer.getHeader());
    os << "\n";
    SerializeToStream(os, dnsViewer.getQuestion());
    os << "\n";
    for (const auto& answer : dnsViewer.getAnswers()) {
        SerializeToStream(os, answer);
        os << "\n";
    }
}

} //! namespace name

namespace posnet {

BadDnsPackage::BadDnsPackage(const std::string_view msg):
m_msg(msg)
{}

const char* BadDnsPackage::what() const noexcept
{
    return m_msg.data();
}

DnsViewer::DnsViewer(ConstRawFrameViewType rawFrame):
BaseFrame(nullptr, 0),
m_header(nullptr),
m_question(nullptr, {}),
m_answers()
{
    auto start = const_cast<ConstRawFrameViewType::value_type*>(rawFrame.data());
    auto queriesDomainSize = 0;
    m_header = HeaderViewer{ reinterpret_cast<HeaderStructType*>(start) };
    // Set up question section
    {
        auto questionDomain = DNSDomainFormatToStr(
                    std::string_view{
                        reinterpret_cast<char*>(start + sizeof(HeaderStructType)),
                        rawFrame.size() - sizeof(HeaderStructType)
                    }
        );
        queriesDomainSize = DnsBuilder::StrToDNSDomainFormat(questionDomain).size();
        m_question = QuestionViewer{
                reinterpret_cast<QuestionStructType*>(start + sizeof(HeaderStructType) + queriesDomainSize),
                std::move(questionDomain)
        };
    }

    // Set up answers section
    {
        auto answerSectionStart = reinterpret_cast<std::uint8_t*>(start + sizeof(HeaderStructType) + queriesDomainSize + sizeof(QuestionStructType));
        auto it = answerSectionStart;
        for (auto i = 0; i < m_header.getAnswerCount(); ++i) {
            std::string domain;

            //! If the first two bytes are equal 0xC0(b'1') it means that the 3 and 4 bytes are offset to domain name.
            //! If the first two bytes are equal 0xC0(b'1') it means that it is domain name and you need to parse the name as using domain name
            const auto offset = ExtractOffsetFromCompressedPointer(it);
            if (offset != 0) {
                domain = DNSDomainFormatToStr(std::string_view{
                                                  reinterpret_cast<char*>(start + offset),
                                                  static_cast<std::string_view::size_type>(rawFrame.size() - offset)
                                              });
            } else {
                domain = DNSDomainFormatToStr(
                            std::string_view{
                                reinterpret_cast<char*>(it),
                                static_cast<std::string_view::size_type>(it - start)
                            }
                );
            }

            AnswerViewer answer{
                reinterpret_cast<DnsViewer::AnswerStructType*>(it + 2) ,
                std::move(domain),
                (uint8_t*)((uint8_t*)it + sizeof(AnswerStructType))
            };
            m_answers.push_back(answer);

            it += sizeof(AnswerStructType) + answer.getRecordDataLength();
        }
    }
}

std::string DnsViewer::AnswerViewer::getDomainName()
{
    return m_domain;
}

DnsViewer::AnswerType DnsViewer::AnswerViewer::getType()
{
    return static_cast<AnswerType>(ntohs(m_answer->atype));
}

DnsViewer::AnswerClass DnsViewer::AnswerViewer::getClass()
{
    return static_cast<AnswerClass>(ntohs(m_answer->aclass));
}

std::uint32_t DnsViewer::AnswerViewer::getTTL()
{
    return ntohl(m_answer->ttl);
}

std::optional<std::string> DnsViewer::AnswerViewer::castRecordDataToIpV4Addr()
{
    try {
        return utils::v4::IpAddrToStr(std::get<0>(m_recordData).sin_addr.s_addr);
    } catch (const std::bad_variant_access& ) {
        return std::nullopt;
    }
}

std::optional<std::string> DnsViewer::AnswerViewer::castRecordDataToIpV6Addr()
{
    try {
        return utils::v6::IpAddrToStr(std::get<1>(m_recordData));
    } catch (const std::bad_variant_access& ) {
        return std::nullopt;
    }
}

std::optional<std::string> DnsViewer::AnswerViewer::castRecordDataToText()
{
    try {
        return std::get<2>(m_recordData);
    } catch (const std::bad_variant_access& ) {
        return std::nullopt;
    }
}

std::uint16_t DnsViewer::AnswerViewer::getRecordDataLength()
{
    return ntohs(m_answer->rdlength);
}

std::string DnsViewer::AnswerViewer::getDomainName() const
{
    return m_domain;
}

DnsViewer::AnswerType DnsViewer::AnswerViewer::getType() const
{
    return static_cast<AnswerType>(ntohs(m_answer->atype));
}

DnsViewer::AnswerClass DnsViewer::AnswerViewer::getClass() const
{
    return static_cast<AnswerClass>(ntohs(m_answer->aclass));
}

std::uint32_t DnsViewer::AnswerViewer::getTTL() const
{
    return ntohl(m_answer->ttl);
}

std::uint16_t DnsViewer::AnswerViewer::getRecordDataLength() const
{
    return ntohs(m_answer->rdlength);
}

std::optional<std::string> DnsViewer::AnswerViewer::castRecordDataToIpV4Addr() const
{
    try {
        return utils::v4::IpAddrToStr(std::get<0>(m_recordData).sin_addr.s_addr);
    } catch (const std::bad_variant_access& ) {
        return std::nullopt;
    }
}

std::optional<std::string> DnsViewer::AnswerViewer::castRecordDataToIpV6Addr() const
{
    try {
        return utils::v6::IpAddrToStr(std::get<1>(m_recordData));
    } catch (const std::bad_variant_access& ) {
        return std::nullopt;
    }
}

std::optional<std::string> DnsViewer::AnswerViewer::castRecordDataToText() const
{
    try {
        return std::get<2>(m_recordData);
    } catch (const std::bad_variant_access& ) {
        return std::nullopt;
    }
}

std::ostream& DnsViewer::AnswerViewer::operator<<(std::ostream& os)
{
    SerializeToStream(os, *this);
    return os;
}

std::ostream& DnsViewer::AnswerViewer::operator<<(std::ostream& os) const
{
    SerializeToStream(os, *this);
    return os;
}

DnsViewer::AnswerViewer::AnswerViewer(AnswerStructType* answer, std::string&& domain, const std::uint8_t* recordDataStart):
m_answer(answer),
m_domain(std::move(domain)),
m_recordData()
{
    try {
        switch (getType()) {
            case AnswerType::HostAddrIpV4: {
                struct sockaddr_in ipAddrStorage;
                std::memcpy(&ipAddrStorage.sin_addr.s_addr, recordDataStart, sizeof(std::uint32_t));
                m_recordData = ipAddrStorage;
                break;
            }
            case AnswerType::HostAddrIpV6: {
                struct sockaddr_in6 ipAddrStorage;
                std::memcpy(&ipAddrStorage.sin6_addr, recordDataStart, sizeof(ipAddrStorage.sin6_addr));
                m_recordData = ipAddrStorage;
                break;
            }
            case AnswerType::Text: {
                std::uint8_t textLength = 0;
                std::memcpy(&textLength, recordDataStart, sizeof(textLength));

                std::string text;
                text.resize(textLength);
                std::memcpy(text.data(), recordDataStart + sizeof(textLength), textLength);
                m_recordData = std::move(text);
                break;
            }
            default: {
                ASSERTION(false, BadDnsPackage, "Unsupported record type format")
                break;
            }
        }
    } catch (const std::bad_variant_access& e) {
        auto errorMsg = "Bad dns answer type: " + std::string(e.what());
        ASSERTION(false, BadDnsPackage, errorMsg)
    }
}

DnsViewer::HeaderViewer::HeaderViewer(HeaderStructType* header):
m_header(header)
{}

unsigned short DnsViewer::HeaderViewer::getId()
{
    return ntohs(m_header->id);
}

DnsViewer::Type DnsViewer::HeaderViewer::getType()
{
    return static_cast<Type>(ntohs(m_header->qr));
}

DnsViewer::OpcodeType DnsViewer::HeaderViewer::getOpcode()
{
    return static_cast<OpcodeType>(ntohs(m_header->opcode));
}

bool DnsViewer::HeaderViewer::getFlagAA()
{
    return static_cast<bool>(m_header->aa);
}

bool DnsViewer::HeaderViewer::getFlagTC()
{
    return static_cast<bool>(m_header->tc);
}

bool DnsViewer::HeaderViewer::getFlagRD()
{
    return static_cast<bool>(m_header->rd);
}

bool DnsViewer::HeaderViewer::getFlagRA()
{
    return static_cast<bool>(m_header->ra);
}

DnsViewer::RecordCodeType DnsViewer::HeaderViewer::getRecordCode()
{
    return static_cast<RecordCodeType>(m_header->rcode);
}

unsigned short DnsViewer::HeaderViewer::getQueryCount()
{
    return ntohs(m_header->qdcount);
}

unsigned short DnsViewer::HeaderViewer::getAnswerCount()
{
    return ntohs(m_header->ancount);
}

unsigned short DnsViewer::HeaderViewer::getAuthorityCount()
{
    return ntohs(m_header->nscount);
}

unsigned short DnsViewer::HeaderViewer::getAdditionalRecordCount()
{
    return ntohs(m_header->arcount);
}

unsigned short DnsViewer::HeaderViewer::getId() const
{
    return ntohs(m_header->id);
}

DnsViewer::Type DnsViewer::HeaderViewer::getType() const
{
    return static_cast<Type>(ntohs(m_header->qr));
}

DnsViewer::OpcodeType DnsViewer::HeaderViewer::getOpcode() const
{
    return static_cast<OpcodeType>(ntohs(m_header->opcode));
}

bool DnsViewer::HeaderViewer::getFlagAA() const
{
    return static_cast<bool>(m_header->aa);
}

bool DnsViewer::HeaderViewer::getFlagTC() const
{
    return static_cast<bool>(m_header->tc);
}

bool DnsViewer::HeaderViewer::getFlagRD() const
{
    return static_cast<bool>(m_header->rd);
}

bool DnsViewer::HeaderViewer::getFlagRA() const
{
    return static_cast<bool>(m_header->ra);
}

DnsViewer::RecordCodeType DnsViewer::HeaderViewer::getRecordCode() const
{
    return static_cast<RecordCodeType>(m_header->rcode);
}

unsigned short DnsViewer::HeaderViewer::getQueryCount() const
{
    return ntohs(m_header->qdcount);
}

unsigned short DnsViewer::HeaderViewer::getAnswerCount() const
{
    return ntohs(m_header->ancount);
}

unsigned short DnsViewer::HeaderViewer::getAuthorityCount() const
{
    return ntohs(m_header->nscount);
}

unsigned short DnsViewer::HeaderViewer::getAdditionalRecordCount() const
{
    return ntohs(m_header->arcount);
}

std::ostream& DnsViewer::HeaderViewer::operator<<(std::ostream& os)
{
    SerializeToStream(os, *this);
    return os;
}

std::ostream& DnsViewer::HeaderViewer::operator<<(std::ostream& os) const
{
    SerializeToStream(os, *this);
    return os;
}

DnsViewer::QuestionViewer::QuestionViewer(QuestionStructType* question, std::string&& domain):
m_domain(std::move(domain)),
m_question(question)
{}

std::string_view DnsViewer::QuestionViewer::getQueryDomain()
{
    return m_domain;
}

DnsViewer::QueryType DnsViewer::QuestionViewer::getQueryType()
{
    return static_cast<QueryType>(ntohs(m_question->qtype));
}

DnsViewer::QueryClass DnsViewer::QuestionViewer::getQueryClass()
{
    return static_cast<QueryClass>(ntohs(m_question->qclass));
}

std::string_view DnsViewer::QuestionViewer::getQueryDomain() const
{
    return m_domain;
}

DnsViewer::QueryType DnsViewer::QuestionViewer::getQueryType() const
{
    return static_cast<QueryType>(ntohs(m_question->qtype));
}

DnsViewer::QueryClass DnsViewer::QuestionViewer::getQueryClass() const
{
    return static_cast<QueryClass>(ntohs(m_question->qclass));
}

std::ostream& DnsViewer::QuestionViewer::operator<<(std::ostream& os)
{
    SerializeToStream(os, *this);
    return os;
}

std::ostream& DnsViewer::QuestionViewer::operator<<(std::ostream& os) const
{
    SerializeToStream(os, *this);
    return os;
}

DnsViewer::HeaderViewer& DnsViewer::getHeader()
{
    return m_header;
}

const DnsViewer::HeaderViewer& DnsViewer::getHeader() const
{
    return m_header;
}

DnsViewer::QuestionViewer& DnsViewer::getQuestion()
{
    return m_question;
}

const DnsViewer::QuestionViewer& DnsViewer::getQuestion() const
{
    return m_question;
}

std::span<DnsViewer::AnswerViewer> DnsViewer::getAnswers()
{
    return m_answers;
}

std::span<const DnsViewer::AnswerViewer> DnsViewer::getAnswers() const
{
    return std::span<const AnswerViewer>{ m_answers };
}

std::string DnsViewer::DNSDomainFormatToStr(const std::string_view domain) {
    std::string_view label = domain;
    std::string storage;
    while (true) {
        const auto labelLength = !label.empty() ? static_cast<int>(label[0]) : -1;
        if (labelLength <= 0 || labelLength > domain.size()) {
            break;
        }

        if (!storage.empty()) {
            storage.push_back('.');
        }

        label = label.substr(1);
        std::copy(label.begin(), label.begin() + labelLength, std::back_inserter(storage));
        if (label.empty()) {
            break;
        }
        label = label.substr(labelLength);
    }

    return storage;
}

std::string_view DnsViewer::QueryTypeToStr(const QueryType qtype)
{
    switch (qtype) {
        case QueryType::HostAddrIpV4: return std::string_view{"HostAddress(V4)"};
        case QueryType::HostAddrIpV6: return std::string_view{"HostAddress(V6)"};
        case QueryType::NameServer: return std::string_view{"NameServer"};
        case QueryType::MailDestination: return std::string_view{"MailDestination"};
        case QueryType::MailForwarder: return std::string_view{"MailForwarder"};
        case QueryType::CanonicalName: return std::string_view{"CanonicalName"};
        case QueryType::StartOfAuthority: return std::string_view{"StartOfAuthority"};
        case QueryType::MailDomainName: return std::string_view{"MailDomainName"};
        case QueryType::MailGroupMember: return std::string_view{"MailGroupMember"};
        case QueryType::MailRenameDomainName: return std::string_view{"MailRenameDomainName"};
        case QueryType::Null: return std::string_view{"Null"};
        case QueryType::WKS: return std::string_view{"WKS"};
        case QueryType::Pointer: return std::string_view{"Pointer"};
        case QueryType::HostInfo: return std::string_view{"HostInfo"};
        case QueryType::MailBoxInfo: return std::string_view{"MailBoxInfo"};
        case QueryType::MailExchange: return std::string_view{"MailExchange"};
        case QueryType::Text: return std::string_view{"TexT"};
        default: return std::string_view{"Unknown"};
    }
}

std::string_view DnsViewer::AnswerTypeToStr(const AnswerType atype)
{
    return QueryTypeToStr(static_cast<QueryType>(atype));
}

std::string_view DnsViewer::QueryClassToStr(const QueryClass qclass)
{
    switch (qclass) {
        case QueryClass::Internet: return std::string_view{"Internet"};
        default: return std::string_view{"Unknown"};
    }
}

std::string_view DnsViewer::AnswerClassToStr(const AnswerClass aclass)
{
    return QueryClassToStr(static_cast<QueryClass>(aclass));
}

std::string_view DnsViewer::OpcodeTypeToStr(const OpcodeType opcode)
{
    switch (opcode) {
        case OpcodeType::StandardQuery: return std::string_view{"StandardQuery"};
        case OpcodeType::InversiveQuery: return std::string_view{"InversiveQuery"};
        case OpcodeType::QueryServerStatus: return std::string_view{"QueryServerStatus"};
        default: return std::string_view{"Unknown"};
    }
}

std::string_view DnsViewer::TypeToStr(const Type type)
{
    switch (type) {
        case Type::Request: return std::string_view{"Request"};
        case Type::Reply: return std::string_view{"Reply"};
        default: return std::string_view{"Unknown"};
    }
}

std::ostream& DnsViewer::operator<<(std::ostream& os)
{
    SerializeToStream(os, *this);
    return os;
}

std::ostream& DnsViewer::operator<<(std::ostream& os) const
{
    SerializeToStream(os, *this);
    return os;
}

std::ostream& operator<<(std::ostream& os, DnsViewer::HeaderViewer& dnsHeaderViewer)
{
    return dnsHeaderViewer.operator<<(os);
}

std::ostream& operator<<(std::ostream& os, const DnsViewer::HeaderViewer& dnsHeaderViewer)
{
    return dnsHeaderViewer.operator<<(os);
}

std::ostream& operator<<(std::ostream& os, DnsViewer::QuestionViewer& dnsQuestionViewer)
{
    return dnsQuestionViewer.operator<<(os);
}

std::ostream& operator<<(std::ostream& os, const DnsViewer::QuestionViewer& dnsQuestionViewer)
{
    return dnsQuestionViewer.operator<<(os);
}

std::ostream& operator<<(std::ostream& os, DnsViewer::AnswerViewer& dnsAnswerViewer)
{
    return dnsAnswerViewer.operator<<(os);
}

std::ostream& operator<<(std::ostream& os, const DnsViewer::AnswerViewer& dnsAnswerViewer)
{
    return dnsAnswerViewer.operator<<(os);
}

std::ostream& operator<<(std::ostream& os, DnsViewer& dnsViewer)
{
    return dnsViewer.operator<<(os);
}

std::ostream& operator<<(std::ostream& os, const DnsViewer& dnsViewer)
{
    return dnsViewer.operator<<(os);
}

} //! namespace posnet

#ifndef VS_DNS_VIEWER_H
#define VS_DNS_VIEWER_H

#include "include/base_frame.h"
#include "include/frame-viewers/udp_viewer.h"

#include <sys/socket.h>

#include <exception>
#include <string>
#include <string_view>
#include <ostream>
#include <vector>
#include <variant>
#include <optional>
#include <cstdint>

// https://habr.com/ru/articles/478652/
// https://datatracker.ietf.org/doc/html/rfc1035
// https://habr.com/ru/articles/66020/

namespace posnet {

class BadDnsPackage final : public std::exception {
public:
    explicit BadDnsPackage(std::string_view msg);
    virtual const char* what() const noexcept;

private:
    std::string m_msg;
};

class DnsViewer final : public BaseFrame {
public:
    static constexpr auto DEFAULT_UDP_PORT = 53;
    static constexpr auto TURN_ON_FLAG = true;
    static constexpr auto TURN_OFF_FLAG = !TURN_ON_FLAG;

    using RawFrameViewType = BaseFrame::RawFrameViewType;
    using ConstRawFrameViewType = BaseFrame::ConstRawFrameViewType;
    
    struct dnshdr final {
        std::uint16_t id;           // unique transaction id
        std::uint8_t rd :1;         // recursion desired
        std::uint8_t tc :1;         // truncated
        std::uint8_t aa :1;         // authoritative address
        std::uint8_t opcode :4;     // request/reply type (std, inversed or so)
        std::uint8_t qr :1;         // record type(request/reply)
        std::uint8_t rcode :4;      // request/reply status code
        std::uint8_t z :3;          // reserved
        std::uint8_t ra :1;         // recursion available
        std::uint16_t qdcount;      // Query count
        std::uint16_t ancount;      // Answer count
        std::uint16_t nscount;      // Authority count
        std::uint16_t arcount;      // Additional count
    };

    struct dnsquestion final {
        //! std::uint8_t* qname; search domain
        std::uint16_t qtype;
        std::uint16_t qclass;
    };

    struct dnsanswer final {
        //! std::uint8_t* qname; search domain
        std::uint16_t atype;
        std::uint16_t aclass;
        std::uint32_t ttl;
        std::uint16_t rdlength;
        //! uint8_t* rdata;
    };

    struct dnsauthority final {
        //! std::uin8_t* name;
        std::uint16_t atype;
        std::uint16_t aclass;
        std::uint32_t ttl;
        std::uint16_t rdlength;
        //! std::uint8_t* rdata;
    };

    enum class Type : std::uint8_t {
        Request = 0,
        Reply = 1,
    };

    enum class OpcodeType : std::uint8_t {
        StandardQuery = 0,
        InversiveQuery = 1,
        QueryServerStatus = 2,
    };

    enum class RecordCodeType : std::uint8_t {
        NoError = 0,
        FormatError = 1,
        ServerFailure = 2,
        NameError = 3,
        NotImplemented = 4,
        Refused = 5
    };

    enum class QueryType : std::uint8_t {
        HostAddrIpV4 = 1,       // A - Host Address(32-Bit IP Address of host or domain)
        NameServer = 2,         // NS - Authoritative name server for the domain
        MailDestination = 3,    // MD
        MailForwarder = 4,      // MF
        CanonicalName = 5,      // CNAME - Canonical domain name for and alias e.g www
        StartOfAuthority = 6,   // SOA - Multiple fields that specify which parts of the naming hiererchy a server implements
        MailDomainName = 7,     // MB
        MailGroupMember = 8,    // MG
        MailRenameDomainName = 9,// MR
        Null = 10,              // null RR
        WKS = 11,               // a well known service description
        Pointer = 12,           // PTR - Symbolic link for a domain. e.g net.firewall.cx points to www.firewall.cx
        HostInfo = 13,          // HINFO - Name of CPU ans OS
        MailBoxInfo = 14,       // MINFO - Info about a mailbox or mail list
        MailExchange = 15,      // MX - 16-bit preference and name of the host that acts as a mail exchange server for a domain e.g mail.firewall.cx
        Text = 16,              // TXT - Uninterpreted string of ASCII text
        HostAddrIpV6 = 28,
        Any = 255,
    };

    enum class QueryClass : std::int8_t {
        Internet = 1,
    };

    using AnswerType = QueryType;
    using AnswerClass = QueryClass;
    using AuthorityType = QueryType;
    using AuthorityClass = QueryClass;

    using HeaderStructType = dnshdr;
    using QuestionStructType = dnsquestion;
    using AnswerStructType = dnsanswer;
    using AuthorityStructType = dnsauthority;

    class HeaderViewer final {
    public:
        HeaderViewer(const HeaderViewer& ) = default;
        HeaderViewer(HeaderViewer&& ) noexcept = default;
        HeaderViewer& operator=(const HeaderViewer& ) = default;
        HeaderViewer& operator=(HeaderViewer&& ) noexcept = default;

        unsigned short getId();
        Type getType();
        OpcodeType getOpcode();
        bool getFlagAA();
        bool getFlagTC();
        bool getFlagRD();
        bool getFlagRA();
        RecordCodeType getRecordCode();
        unsigned short getQueryCount();
        unsigned short getAnswerCount();
        unsigned short getAuthorityCount();
        unsigned short getAdditionalRecordCount();

        unsigned short getId() const;
        Type getType() const;
        OpcodeType getOpcode() const;
        bool getFlagAA() const;
        bool getFlagTC() const;
        bool getFlagRD() const;
        bool getFlagRA() const;
        RecordCodeType getRecordCode() const;
        unsigned short getQueryCount() const;
        unsigned short getAnswerCount() const;
        unsigned short getAuthorityCount() const;
        unsigned short getAdditionalRecordCount() const;

        std::ostream& operator<<(std::ostream& os);
        std::ostream& operator<<(std::ostream& os) const;

    private:
        friend DnsViewer;
        explicit HeaderViewer(HeaderStructType* header);
        HeaderStructType* m_header;
    };

    class QuestionViewer final {
    public:
        QuestionViewer(const QuestionViewer& ) = default;
        QuestionViewer(QuestionViewer&& ) noexcept = default;
        QuestionViewer& operator=(const QuestionViewer& ) = default;
        QuestionViewer& operator=(QuestionViewer&& ) noexcept = default;

        std::string_view getQueryDomain();
        QueryType getQueryType();
        QueryClass getQueryClass();

        std::string_view getQueryDomain() const;
        QueryType getQueryType() const;
        QueryClass getQueryClass() const;

        std::ostream& operator<<(std::ostream& os);
        std::ostream& operator<<(std::ostream& os) const;

    private:
        explicit QuestionViewer(QuestionStructType* question, std::string&& domain);
        friend DnsViewer;
        std::string m_domain;
        QuestionStructType* m_question;
    };

    class AnswerViewer final {
    public:
        AnswerViewer(const AnswerViewer& ) = default;
        AnswerViewer(AnswerViewer&& ) noexcept = default;
        AnswerViewer& operator=(const AnswerViewer& ) = default;
        AnswerViewer& operator=(AnswerViewer&& ) noexcept = default;

        using RecordVariantType = std::variant<
            struct sockaddr_in, // A (Host address (IpV4))
            struct sockaddr_in6, // AAAA (Host Address (IpV6))
            std::string // (TXT)text
        >;

        std::string getDomainName();
        AnswerType getType();
        AnswerClass getClass();
        std::uint32_t getTTL();
        std::uint16_t getRecordDataLength();
        std::optional<std::string> castRecordDataToIpV4Addr();
        std::optional<std::string> castRecordDataToIpV6Addr();
        std::optional<std::string> castRecordDataToText();

        std::string getDomainName() const;
        AnswerType getType() const;
        AnswerClass getClass() const;
        std::uint32_t getTTL() const;
        std::uint16_t getRecordDataLength() const;
        std::optional<std::string> castRecordDataToIpV4Addr() const;
        std::optional<std::string> castRecordDataToIpV6Addr() const;
        std::optional<std::string> castRecordDataToText() const;

        std::ostream& operator<<(std::ostream& os);
        std::ostream& operator<<(std::ostream& os) const;

    private:
        explicit AnswerViewer(AnswerStructType* answer, std::string&& domain, const std::uint8_t* recordDataStart);
        friend DnsViewer;
        AnswerStructType* m_answer;
        std::string m_domain;
        RecordVariantType m_recordData;
    };

    explicit DnsViewer(ConstRawFrameViewType rawFrame);
    explicit DnsViewer(UdpViewer udpViewer);

    HeaderViewer& getHeader();
    const HeaderViewer& getHeader() const;

    QuestionViewer& getQuestion();
    const QuestionViewer& getQuestion() const;

    std::span<AnswerViewer> getAnswers();
    std::span<const AnswerViewer> getAnswers() const;

    std::ostream& operator<<(std::ostream& os);
    std::ostream& operator<<(std::ostream& os) const;

    static std::string DNSDomainFormatToStr(std::string_view domain);
    static std::string_view QueryTypeToStr(QueryType qtype);
    static std::string_view AnswerTypeToStr(AnswerType atype);
    static std::string_view QueryClassToStr(QueryClass qclass);
    static std::string_view AnswerClassToStr(AnswerClass aclass);
    static std::string_view OpcodeTypeToStr(OpcodeType opcode);
    static std::string_view TypeToStr(Type type);

private:
    HeaderViewer m_header;
    QuestionViewer m_question;
    std::vector<AnswerViewer> m_answers;
};

std::ostream& operator<<(std::ostream& os, DnsViewer::HeaderViewer& dnsHeaderViewer);
std::ostream& operator<<(std::ostream& os, const DnsViewer::HeaderViewer& dnsHeaderViewer);

std::ostream& operator<<(std::ostream& os, DnsViewer::QuestionViewer& dnsQuestionViewer);
std::ostream& operator<<(std::ostream& os, const DnsViewer::QuestionViewer& dnsQuestionViewer);

std::ostream& operator<<(std::ostream& os, DnsViewer::AnswerViewer& dnsAnswerViewer);
std::ostream& operator<<(std::ostream& os, const DnsViewer::AnswerViewer& dnsAnswerViewer);

std::ostream& operator<<(std::ostream& os, DnsViewer& dnsViewer);
std::ostream& operator<<(std::ostream& os, const DnsViewer& dnsViewer);

} //! namespace posnet

#endif //! VS_DNS_VIEWER_H

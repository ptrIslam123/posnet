#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>

#define MODULE_NAME "net_logger"

static struct nf_hook_ops nfho;

static void parse_tcp_header(const struct tcphdr* header)
{
    printk(KERN_INFO MODULE_NAME": TCP Header Info:\n");
    printk(KERN_INFO MODULE_NAME":   - Source Port: %d\n", ntohs(header->source));
    printk(KERN_INFO MODULE_NAME":   - Destination Port: %d\n", ntohs(header->dest));
    printk(KERN_INFO MODULE_NAME":   - Sequence Number: %u\n", ntohl(header->seq));
    printk(KERN_INFO MODULE_NAME":   - Acknowledgment Number: %u\n", ntohl(header->ack_seq));
}

static void parse_udp_header(const struct udphdr* header)
{
    printk(KERN_INFO "UDP Header Info:\n");
    printk(KERN_INFO "  - Source Port: %d\n", ntohs(header->source));
    printk(KERN_INFO "  - Destination Port: %d\n", ntohs(header->dest));
}

static void parse_icmp_header(const struct icmphdr* header)
{
    printk(KERN_INFO MODULE_NAME": ICMP Header Info:\n");
    printk(KERN_INFO MODULE_NAME":   - Type: %d\n", header->type);
    printk(KERN_INFO MODULE_NAME":   - Code: %d\n", header->code);
}

static void parse_ip_header(const struct iphdr* header)
{
    char src_ip[16];
    char dst_ip[16];

    snprintf(src_ip, sizeof(src_ip), "%pI4", &header->saddr);
    snprintf(dst_ip, sizeof(dst_ip), "%pI4", &header->daddr);

    printk(KERN_INFO MODULE_NAME": IP Header Info:\n");
    printk(KERN_INFO MODULE_NAME":   - Source IP: %s\n", src_ip);
    printk(KERN_INFO MODULE_NAME":   - Destination IP: %s\n", dst_ip);
    printk(KERN_INFO MODULE_NAME":   - Protocol: %d\n", header->protocol);
    printk(KERN_INFO MODULE_NAME":   - TTL: %d\n", header->ttl);
}

static unsigned int hook_entrypoint(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) 
{
    if (!skb || skb->protocol != htons(ETH_P_IP)) {
        return NF_ACCEPT;
    }

    struct iphdr *iph;
    struct tcphdr *tcph;
    struct udphdr *udph;
    struct icmphdr *icmph;

    iph = ip_hdr(skb);

    // Check if the packet is an IPv4 packet
    if (iph->version != 4) {
        printk(KERN_INFO MODULE_NAME": Not an IPv4 packet(skip this package)\n");
        return NF_ACCEPT;
    }

    parse_ip_header(iph);

    // Check the protocol and handle accordingly
    switch (iph->protocol) {
        case IPPROTO_TCP:
            tcph = (struct tcphdr *)((__u32 *)iph + iph->ihl);
            parse_tcp_header(tcph);
            break;
        case IPPROTO_UDP:
            udph = (struct udphdr *)((__u32 *)iph + iph->ihl);
            parse_udp_header(udph);
            break;
        case IPPROTO_ICMP:
            icmph = (struct icmphdr *)((__u32 *)iph + iph->ihl);
            parse_icmp_header(icmph);
            break;
        default:
            printk(KERN_INFO MODULE_NAME": Other IP Packet(skip this package)\n");
            break;
    }

    return NF_ACCEPT;
}

static int __init init_module_device(void) 
{
    printk(KERN_INFO MODULE_NAME": Init module\n");

    nfho.hook = hook_entrypoint;
    nfho.hooknum = NF_INET_PRE_ROUTING;
    nfho.pf = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &nfho);
    return 0;
}

static void __exit deinit_module_device(void) 
{
    printk(KERN_INFO MODULE_NAME": Deinit module");
    nf_unregister_net_hook(&init_net, &nfho);
}

module_init(init_module_device);
module_exit(deinit_module_device);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kardanov.I");
MODULE_DESCRIPTION("A simple net package logger");
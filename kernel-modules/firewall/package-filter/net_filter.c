#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>

#include "net_filter_config.h"

#define MODULE_NAME "net_filter"
#define DEVICE_COUNT (1)
#define DEVICE_BASE_MINOR (0)
#define DRIVER_NAME (MODULE_NAME)
#define DEVICE_NAME (MODULE_NAME)
#define DEVICE_CLASS_NAME (MODULE_NAME)

static dev_t char_dev_num; // Global variable for the first device number
static struct cdev char_dev; // Global variable for the character device structure
static struct class *char_dev_class; // Global variable for the device class
static struct package_filter_config config; 

static long device_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
    printk(KERN_INFO MODULE_NAME": Device ioctl operation\n");
    if ((_IOC_TYPE(cmd) != MY_IOC_MAGIC)) {
		return -EINVAL;
	}

    switch (cmd) {
        case PACKAGE_FILTER_S_CONFIG: {
            printk(KERN_INFO MODULE_NAME": Set new configuration\n");
            struct package_filter_config tmp_conf;
            struct package_filter_config* user_config = (struct package_filter_config*)(arg);
            if (copy_from_user(&tmp_conf, user_config, sizeof(tmp_conf)) < 0) {
                printk(KERN_INFO MODULE_NAME": Invalid user memory\n");
                return -EINVAL;
            }

            tmp_conf.package_desc = kzalloc(sizeof(struct package_description) * user_config->size, GFP_KERNEL);
            if (!tmp_conf.package_desc) {
                printk(KERN_INFO MODULE_NAME": Could not allocate memory to store filter configuration\n");
                return -ENOMEM;
            }
            tmp_conf.size = user_config->size;

            if (copy_from_user(tmp_conf.package_desc, user_config->package_desc, sizeof(struct package_description) * tmp_conf.size) < 0) {
                printk(KERN_INFO MODULE_NAME": Invalid user memory\n");
                return -EINVAL;
            }

            if (config.size > 0) {
                kfree(config.package_desc);
            }

            memcpy(config.package_desc, tmp_conf.package_desc, sizeof(struct package_description) * tmp_conf.size);
            config.size = tmp_conf.size;
            break;
        }
        default: {
            printk(KERN_INFO MODULE_NAME": Invalid cmd=%d\n", cmd);
            return -EINVAL;
        }
    }
    return 0;
}

static const struct file_operations char_file_ops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = device_ioctl,
};

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
    printk(KERN_INFO MODULE_NAME": UDP Header Info:\n");
    printk(KERN_INFO MODULE_NAME":   - Source Port: %d\n", ntohs(header->source));
    printk(KERN_INFO MODULE_NAME":   - Destination Port: %d\n", ntohs(header->dest));
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

    // Check the protocol and handle accordingly
    switch (iph->protocol) {
        case IPPROTO_TCP:
            tcph = (struct tcphdr *)((__u32 *)iph + iph->ihl);
            
            break;
        case IPPROTO_UDP:
            udph = (struct udphdr *)((__u32 *)iph + iph->ihl);
            
            break;
        default:
            break;
    }

    return NF_ACCEPT;
}

static int init_chdev(void)
{
    int ret;

    //! int alloc_chrdev_region(dev_t * dev, unsigned baseminor, unsigned count, const char * name); - Allocate device numbers dynamically
    ret = alloc_chrdev_region(&char_dev_num, DEVICE_BASE_MINOR, DEVICE_COUNT, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT MODULE_NAME": Failed to allocate device number\n");
        return ret;
    }

    //! void cdev_init(struct cdev *, const struct file_operations *); - Initialize the character device structure
    cdev_init(&char_dev, &char_file_ops);

    //! int cdev_add(struct cdev *, dev_t, unsigned); - Add the character device to the system
    ret = cdev_add(&char_dev, char_dev_num, DEVICE_COUNT);
    if (ret < 0) {
        unregister_chrdev_region(DEVICE_BASE_MINOR, DEVICE_COUNT);
        printk(KERN_ALERT MODULE_NAME": Failed to add device\n");
        return ret;
    }

    //! #define class_create(owner, name) - Create a class
    char_dev_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME);
    if (IS_ERR(char_dev_class)) {
        cdev_del(&char_dev);
        unregister_chrdev_region(DEVICE_BASE_MINOR, DEVICE_COUNT);
        printk(KERN_ALERT MODULE_NAME": Failed to create class\n");
        return PTR_ERR(char_dev_class);
    }

    //! struct device *device_create(struct class *cls, struct device *parent, dev_t devt, void *drvdata, const char *fmt, ...); - Create a device node
    device_create(char_dev_class, NULL, char_dev_num, NULL, DEVICE_NAME);
    return 0;
}

static int init_netfilter(void)
{
    nfho.hook = hook_entrypoint;
    nfho.hooknum = NF_INET_PRE_ROUTING;
    nfho.pf = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &nfho);
    return 0;
}

static int __init init_module_device(void) 
{
    printk(KERN_INFO MODULE_NAME": Init module\n");
    if (init_chdev() < 0) {
        return -1;
    }

    if (init_netfilter() < 0) {
        return -1;
    }
    
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
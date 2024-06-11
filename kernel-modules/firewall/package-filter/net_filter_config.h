#ifndef POSNET_PACKAGE_FILTER_CONFIG_H
#define POSNET_PACKAGE_FILTER_CONFIG_H

#include </usr/include/asm-generic/ioctl.h>
#include <linux/types.h>

#define MY_IOC_MAGIC ('h')

typedef enum package_status 
{
    PACKAGE_FILTER_STATUS_ACCEPT,
    PACKAGE_FILTER_STATUS_DROPT,
} package_status_t;

typedef struct package_description 
{
    __u32 ip_addr;
    __u16 port;
    enum package_status status;
} package_description_t;

typedef struct package_filter_config 
{
    int size;
    struct package_description* package_desc;
} package_filter_config_t;

#define PACKAGE_FILTER_S_CONFIG (_IOW(MY_IOC_MAGIC, 1, package_filter_config_t))

#endif //! POSNET_PACKAGE_FILTER_CONFIG_H

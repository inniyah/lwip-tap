#ifndef __TAPIF_HELPERS_H__
#define __TAPIF_HELPERS_H__

#include "tapif.h"
#include <netdb.h>
#include <netinet/in.h>

int parse_address(char * addr, struct addrinfo * info, int family);
int parse_pair(struct tapif * tapif, char * key, char * value);
int parse_interface(struct tapif * tapif, char* param);

#endif /* __TAPIF_HELPERS_H__ */

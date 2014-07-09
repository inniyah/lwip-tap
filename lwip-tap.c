/*
 * Copyright (c) 2012-2013 Takayuki Usui
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "chargen.h"
#include "httpserver-netconn.h"
#include "tapif.h"
#include "tapif_helpers.h"
#include "tcpecho.h"
#include "udpecho.h"

#define NETIF_MAX 64

void help(void) {
#ifdef LWIP_DEBUG
  fprintf(stderr,"Usage: lwip-tap [-CEHdh] -i addr=<addr>,netmask=<addr>,name=<name>,gw=<addr> [...]\n");
#else
  fprintf(stderr,"Usage: lwip-tap [-CEHh] -i addr=<addr>,netmask=<addr>,name=<name>,gw=<addr> [...]\n");
#endif
  exit(0);
}

#define IP4_OR_NULL(ip_addr) ((ip_addr).addr == IPADDR_ANY ? 0 : &(ip_addr))

int main(int argc, char *argv[]) {
  struct tapif tapif[NETIF_MAX];
  struct netif netif[NETIF_MAX];
  int ch;
  int n = 0;

  memset(tapif,0,sizeof(tapif));
  memset(netif,0,sizeof(netif));

  tcpip_init(NULL,NULL);

#ifdef LWIP_DEBUG
  while ((ch = getopt(argc,argv,"CEHdhi:")) != -1) {
#else
  while ((ch = getopt(argc,argv,"CEHhi:")) != -1) {
#endif
    switch (ch) {
    case 'C':
      chargen_init();
      break;
    case 'E':
      udpecho_init();
      tcpecho_init();
      break;
    case 'H':
      http_server_netconn_init();
      break;
#ifdef LWIP_DEBUG
    case 'd':
      debug_flags |= (LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|
                      LWIP_DBG_FRESH|LWIP_DBG_HALT);
      break;
#endif
    case 'i':
      if (n >= NETIF_MAX)
        break;
      if (parse_interface(&tapif[n],optarg) != 0)
        help();
      netif_add(&netif[n],
                IP4_OR_NULL(tapif[n].ip_addr),
                IP4_OR_NULL(tapif[n].netmask),
                IP4_OR_NULL(tapif[n].gw),
                &tapif[n],
                tapif_init,
                tcpip_input);
      if (n == 0)
        netif_set_default(&netif[n]);
      netif_set_up(&netif[n]);
      if (IP4_OR_NULL(tapif[n].ip_addr) == 0 &&
          IP4_OR_NULL(tapif[n].netmask) == 0 &&
          IP4_OR_NULL(tapif[n].gw) == 0)
        dhcp_start(&netif[n]);
      n++;
      break;
    case 'h':
    default:
      help();
    }
  }
  argc -= optind;
  argv += optind;
  if (n <= 0) {
      help();
  }
  pause();
  return -1;
}

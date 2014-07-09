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

#include "tapif_helpers.h"
#include "tapif.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int parse_address(char* addr, struct addrinfo* info,int family) {
  struct addrinfo hints;
  struct addrinfo *result;
  struct addrinfo *r;
  int res;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = 0;
  hints.ai_protocol = 0;
  hints.ai_flags = AI_NUMERICSERV;
  res = getaddrinfo(addr,0,&hints,&result);
  if (res != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
    return -1;
  }
  for (r = result; r != NULL; r = r->ai_next) {
    if (r->ai_family == family) {
      *info = *r;
      return 0;
    }
  }
  return -1;
}

int parse_pair(struct tapif* tapif,char* key, char* value) {
  struct addrinfo addr;
  uint8_t* p;

  if (key == 0 || *key == 0)
    return -1;

#define PARSE_IP4(ip_addr)                                        \
  do {                                                            \
    if (value == 0 || *value == 0)                                \
      return -1;                                                  \
    if (parse_address(value,&addr,AF_INET) != 0)                  \
      return -1;                                                  \
    p = (uint8_t*)&((struct sockaddr_in*)addr.ai_addr)->sin_addr; \
    IP4_ADDR(&(ip_addr),*p,*(p+1),*(p+2),*(p+3));                 \
    return 0;                                                     \
  } while(0)

  if (!strcmp(key,"name")) {
    if (value == 0 || *value == 0)
      return -1;
    tapif->name = value;
    return 0;
  } else if (!strcmp(key,"addr")) {
    PARSE_IP4(tapif->ip_addr);
  } else if (!strcmp(key,"netmask")) {
    PARSE_IP4(tapif->netmask);
  } else if (!strcmp(key,"gw")) {
    PARSE_IP4(tapif->gw);
  } else {
    return -1;
  }
}

int parse_interface(struct tapif* tapif, char* param) {
  enum {
    KEY_WAIT, KEY, VALUE_WAIT, VALUE, END
  };
  int state = KEY_WAIT;
  char* p = param;
  char* key = 0;
  char* value = 0;

  while (state != END) {
    switch (*p) {
    case '\0':
      if (parse_pair(tapif,key,value) != 0)
        return -1;
      state = END;
      break;
    case ',':
      if (state == KEY_WAIT) {
        p++;
        break;
      }
      state = KEY_WAIT;
      *p++ = 0;
      if (parse_pair(tapif,key,value) != 0)
        return -1;
      key = value = 0;
      break;
    case '=':
      if (state == KEY)
        state = VALUE_WAIT;
      else
        return -1;
      *p++ = 0;
      break;
    case ' ':
      if (state != KEY_WAIT && state != VALUE_WAIT)
        return -1;
      p++;
      break;
    default:
      if (state == KEY_WAIT) {
        state = KEY;
        key = p;
      } else if (state == VALUE_WAIT) {
        state = VALUE;
        value = p;
      }
      p++;
    }
  }
  return 0;
}

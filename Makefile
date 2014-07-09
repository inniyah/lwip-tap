LIBNAME=lwip-tap
LIBRARY=lib$(LIBNAME)

all: $(LIBRARY).a $(LIBRARY).so lwip-tap

MAJOR=0
MINOR=0

LIB_SOURCES = \
  lwip/src/api/api_lib.c \
  lwip/src/api/api_msg.c \
  lwip/src/api/err.c \
  lwip/src/api/netbuf.c \
  lwip/src/api/netdb.c \
  lwip/src/api/netifapi.c \
  lwip/src/api/pppapi.c \
  lwip/src/api/sockets.c \
  lwip/src/api/tcpip.c \
  lwip/src/core/def.c \
  lwip/src/core/dhcp.c \
  lwip/src/core/dns.c \
  lwip/src/core/inet_chksum.c \
  lwip/src/core/init.c \
  lwip/src/core/mem.c \
  lwip/src/core/memp.c \
  lwip/src/core/netif.c \
  lwip/src/core/pbuf.c \
  lwip/src/core/raw.c \
  lwip/src/core/stats.c \
  lwip/src/core/sys.c \
  lwip/src/core/tcp.c \
  lwip/src/core/tcp_in.c \
  lwip/src/core/tcp_out.c \
  lwip/src/core/timers.c \
  lwip/src/core/udp.c \
  lwip/src/core/ipv4/autoip.c \
  lwip/src/core/ipv4/icmp.c \
  lwip/src/core/ipv4/igmp.c \
  lwip/src/core/ipv4/ip4.c \
  lwip/src/core/ipv4/ip4_addr.c \
  lwip/src/core/ipv4/ip_frag.c \
  lwip/src/core/ipv6/inet6.c \
  lwip/src/core/ipv6/dhcp6.c \
  lwip/src/core/ipv6/ip6_frag.c \
  lwip/src/core/ipv6/ethip6.c \
  lwip/src/core/ipv6/nd6.c \
  lwip/src/core/ipv6/mld6.c \
  lwip/src/core/ipv6/ip6_addr.c \
  lwip/src/core/ipv6/icmp6.c \
  lwip/src/core/ipv6/ip6.c \
  lwip/src/core/snmp/asn1_dec.c \
  lwip/src/core/snmp/asn1_enc.c \
  lwip/src/core/snmp/mib2.c \
  lwip/src/core/snmp/mib_structs.c \
  lwip/src/core/snmp/msg_in.c \
  lwip/src/core/snmp/msg_out.c \
  lwip/src/netif/etharp.c \
  lwip/src/netif/ethernetif.c \
  lwip/src/netif/slipif.c \
  lwip-contrib/ports/unix/sys_arch.c \
  lwip-contrib/apps/chargen/chargen.c \
  lwip-contrib/apps/httpserver/httpserver-netconn.c \
  lwip-contrib/apps/tcpecho/tcpecho.c \
  lwip-contrib/apps/udpecho/udpecho.c \
  tapif.c \
  debug_flags.c

LIB_SHARED_OBJS = $(LIB_SOURCES:.c=.shared.o)
LIB_STATIC_OBJS = $(LIB_SOURCES:.c=.static.o)

PKG_CONFIG=
PKG_CONFIG_CFLAGS=`pkg-config --cflags $(PKG_CONFIG) 2>/dev/null`
PKG_CONFIG_LIBS=`pkg-config --libs $(PKG_CONFIG) 2>/dev/null`

INCLUDES=-I. -Ilwip-contrib/ports/unix/include \
	-Ilwip/src/include/ipv4 -Ilwip/src/include/ipv6 -Ilwip/src/include \
	-Ilwip-contrib/apps/chargen -Ilwip-contrib/apps/httpserver \
	-Ilwip-contrib/apps/tcpecho -Ilwip-contrib/apps/udpecho

DEFINES=-DLWIP_DEBUG=1

CFLAGS=-O2 -g -Wall
STATIC_CFLAGS= $(CFLAGS) $(DEFINES) $(INCLUDES) $(CFLAGS) $(PKG_CONFIG_CFLAGS)
SHARED_CFLAGS= $(STATIC_CFLAGS) -fPIC

LDFLAGS= -Wl,-z,defs -Wl,--as-needed -Wl,--no-undefined
LIBS=-lpthread

lwip-tap: libraries lwip-tap.c
	gcc $(STATIC_CFLAGS) -I. $(LDFLAGS) $(EXTRA_LDFLAGS) lwip-tap.c -o $@ -l$(LIBNAME) -L. $(LIBS) $(PKG_CONFIG_LIBS)

libraries: $(LIBRARY).so $(LIBRARY).a

$(LIBRARY).so.$(MAJOR).$(MINOR): $(LIB_SHARED_OBJS)
	g++ $(LDFLAGS) $(EXTRA_LDFLAGS) -shared \
		-Wl,-soname,$(LIBRARY).so.$(MAJOR) \
		-o $(LIBRARY).so.$(MAJOR).$(MINOR) \
		$+ -o $@ $(LIBS) $(PKG_CONFIG_LIBS)

$(LIBRARY).so: $(LIBRARY).so.$(MAJOR).$(MINOR)
	rm -f $@.$(MAJOR)
	ln -s $@.$(MAJOR).$(MINOR) $@.$(MAJOR)
	rm -f $@
	ln -s $@.$(MAJOR) $@

$(LIBRARY).a: $(LIB_STATIC_OBJS)
	ar cru $@ $+

%.shared.o: %.cpp
	g++ -o $@ -c $+ $(SHARED_CFLAGS)

%.shared.o: %.c
	gcc -o $@ -c $+ $(SHARED_CFLAGS)

%.so : %.o
	g++ $(LDFLAGS) $(EXTRA_LDFLAGS) -shared $^ -o $@

%.static.o: %.cpp
	g++ -o $@ -c $+ $(STATIC_CFLAGS)

%.static.o: %.c
	gcc -o $@ -c $+ $(STATIC_CFLAGS)

.depend depend dep:
	gcc $(SHARED_CFLAGS) -MM $(LIB_SOURCES) >.depend

clean:
	rm -f $(LIB_SHARED_OBJS)
	rm -f $(LIB_STATIC_OBJS)
	rm -f *.so *.so* *.a *~

test: lwip-tap
	sudo LD_LIBRARY_PATH="`pwd`" ./lwip-tap -i addr=172.16.0.2,netmask=255.255.255.0,gw=172.16.0.1

DESTDIR=
MULTIARCH=

install: $(LIBRARY).a $(LIBRARY).so
	mkdir -p "$(DESTDIR)/usr/lib/$(MULTIARCH)/"
	cp -a *.a "$(DESTDIR)/usr/lib/$(MULTIARCH)/"
	cp -a *.so* "$(DESTDIR)/usr/lib/$(MULTIARCH)/"

ifeq (.depend,$(wildcard .depend))
include .depend
endif

.PHONY: all depend dep clean install

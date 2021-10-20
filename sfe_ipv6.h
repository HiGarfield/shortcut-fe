/*
 * sfe_ipv6.h
 *	Shortcut forwarding engine header file for IPv6.
 *
 * Copyright (c) 2015-2016, 2019-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define CHAR_DEV_MSG_SIZE 768

#define SFE_IPV6_DSCP_MASK 0xf03f
#define SFE_IPV6_DSCP_SHIFT 2

#define	SFE_IPV6_FRAG_OFFSET	0xfff8

/*
 * generic IPv6 extension header
 */
struct sfe_ipv6_ext_hdr {
	__u8 next_hdr;
	__u8 hdr_len;
	__u8 padding[6];
};

/*
 * Specifies the lower bound on ACK numbers carried in the TCP header
 */
#define SFE_IPV6_TCP_MAX_ACK_WINDOW 65520

/*
 * IPv6 TCP connection match additional data.
 */
struct sfe_ipv6_tcp_connection_match {
	u8 win_scale;		/* Window scale */
	u32 max_win;		/* Maximum window size seen */
	u32 end;		/* Sequence number of the next byte to send (seq + segment length) */
	u32 max_end;		/* Sequence number of the last byte to ack */
};

/*
 * Bit flags for IPv6 connection matching entry.
 */
#define SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_SRC (1<<0)
					/* Perform source translation */
#define SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_DEST (1<<1)
					/* Perform destination translation */
#define SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK (1<<2)
					/* Ignore TCP sequence numbers */
#define SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR (1<<3)
					/* Fast Ethernet header write */
#define SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_L2_HDR (1<<4)
					/* Fast Ethernet header write */
#define SFE_IPV6_CONNECTION_MATCH_FLAG_PRIORITY_REMARK (1<<5)
					/* remark priority of SKB */
#define SFE_IPV6_CONNECTION_MATCH_FLAG_DSCP_REMARK (1<<6)
					/* remark DSCP of packet */

/*
 * IPv6 connection matching structure.
 */
struct sfe_ipv6_connection_match {
	/*
	 * References to other objects.
	 */
	struct sfe_ipv6_connection_match *next;
	struct sfe_ipv6_connection_match *prev;
	struct sfe_ipv6_connection *connection;
	struct sfe_ipv6_connection_match *counter_match;
					/* Matches the flow in the opposite direction as the one in connection */
	struct sfe_ipv6_connection_match *active_next;
	struct sfe_ipv6_connection_match *active_prev;
	bool active;			/* Flag to indicate if we're on the active list */

	/*
	 * Characteristics that identify flows that match this rule.
	 */
	struct net_device *match_dev;	/* Network device */
	u8 match_protocol;		/* Protocol */
	struct sfe_ipv6_addr match_src_ip[1];	/* Source IP address */
	struct sfe_ipv6_addr match_dest_ip[1];	/* Destination IP address */
	__be16 match_src_port;		/* Source port/connection ident */
	__be16 match_dest_port;		/* Destination port/connection ident */

	/*
	 * Control the operations of the match.
	 */
	u32 flags;			/* Bit flags */
#ifdef CONFIG_NF_FLOW_COOKIE
	u32 flow_cookie;		/* used flow cookie, for debug */
#endif
#ifdef CONFIG_XFRM
	u32 flow_accel;            	/* The flow accelerated or not */
#endif

	/*
	 * Connection state that we track once we match.
	 */
	union {				/* Protocol-specific state */
		struct sfe_ipv6_tcp_connection_match tcp;
	} protocol_state;
	/*
	 * Stats recorded in a sync period. These stats will be added to
	 * rx_packet_count64/rx_byte_count64 after a sync period.
	 */
	u32 rx_packet_count;
	u32 rx_byte_count;

	/*
	 * Packet translation information.
	 */
	struct sfe_ipv6_addr xlate_src_ip[1];	/* Address after source translation */
	__be16 xlate_src_port;	/* Port/connection ident after source translation */
	u16 xlate_src_csum_adjustment;
					/* Transport layer checksum adjustment after source translation */
	struct sfe_ipv6_addr xlate_dest_ip[1];	/* Address after destination translation */
	__be16 xlate_dest_port;	/* Port/connection ident after destination translation */
	u16 xlate_dest_csum_adjustment;
					/* Transport layer checksum adjustment after destination translation */

	/*
	 * QoS information
	 */
	u32 priority;
	u32 dscp;

	/*
	 * Packet transmit information.
	 */
	struct net_device *xmit_dev;	/* Network device on which to transmit */
	unsigned short int xmit_dev_mtu;
					/* Interface MTU */
	u16 xmit_dest_mac[ETH_ALEN / 2];
					/* Destination MAC address to use when forwarding */
	u16 xmit_src_mac[ETH_ALEN / 2];
					/* Source MAC address to use when forwarding */

	/*
	 * Summary stats.
	 */
	u64 rx_packet_count64;
	u64 rx_byte_count64;
};

/*
 * Per-connection data structure.
 */
struct sfe_ipv6_connection {
	struct sfe_ipv6_connection *next;
					/* Pointer to the next entry in a hash chain */
	struct sfe_ipv6_connection *prev;
					/* Pointer to the previous entry in a hash chain */
	int protocol;			/* IP protocol number */
	struct sfe_ipv6_addr src_ip[1];		/* Src IP addr pre-translation */
	struct sfe_ipv6_addr src_ip_xlate[1];	/* Src IP addr post-translation */
	struct sfe_ipv6_addr dest_ip[1];	/* Dest IP addr pre-translation */
	struct sfe_ipv6_addr dest_ip_xlate[1];	/* Dest IP addr post-translation */
	__be16 src_port;		/* Src port pre-translation */
	__be16 src_port_xlate;		/* Src port post-translation */
	__be16 dest_port;		/* Dest port pre-translation */
	__be16 dest_port_xlate;		/* Dest port post-translation */
	struct sfe_ipv6_connection_match *original_match;
					/* Original direction matching structure */
	struct net_device *original_dev;
					/* Original direction source device */
	struct sfe_ipv6_connection_match *reply_match;
					/* Reply direction matching structure */
	struct net_device *reply_dev;	/* Reply direction source device */
	u64 last_sync_jiffies;		/* Jiffies count for the last sync */
	struct sfe_ipv6_connection *all_connections_next;
					/* Pointer to the next entry in the list of all connections */
	struct sfe_ipv6_connection *all_connections_prev;
					/* Pointer to the previous entry in the list of all connections */
	u32 mark;			/* mark for outgoing packet */
	u32 debug_read_seq;		/* sequence number for debug dump */
};

/*
 * IPv6 connections and hash table size information.
 */
#define SFE_IPV6_CONNECTION_HASH_SHIFT 12
#define SFE_IPV6_CONNECTION_HASH_SIZE (1 << SFE_IPV6_CONNECTION_HASH_SHIFT)
#define SFE_IPV6_CONNECTION_HASH_MASK (SFE_IPV6_CONNECTION_HASH_SIZE - 1)

enum sfe_ipv6_exception_events {
	SFE_IPV6_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE,
	SFE_IPV6_EXCEPTION_EVENT_UDP_NO_CONNECTION,
	SFE_IPV6_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT,
	SFE_IPV6_EXCEPTION_EVENT_UDP_SMALL_TTL,
	SFE_IPV6_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION,
	SFE_IPV6_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE,
	SFE_IPV6_EXCEPTION_EVENT_TCP_NO_CONNECTION_SLOW_FLAGS,
	SFE_IPV6_EXCEPTION_EVENT_TCP_NO_CONNECTION_FAST_FLAGS,
	SFE_IPV6_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT,
	SFE_IPV6_EXCEPTION_EVENT_TCP_SMALL_TTL,
	SFE_IPV6_EXCEPTION_EVENT_TCP_NEEDS_FRAGMENTATION,
	SFE_IPV6_EXCEPTION_EVENT_TCP_FLAGS,
	SFE_IPV6_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE,
	SFE_IPV6_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS,
	SFE_IPV6_EXCEPTION_EVENT_TCP_BAD_SACK,
	SFE_IPV6_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS,
	SFE_IPV6_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE,
	SFE_IPV6_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE,
	SFE_IPV6_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE,
	SFE_IPV6_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE,
	SFE_IPV6_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE,
	SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_HEADER_INCOMPLETE,
	SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_NON_V6,
	SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_IP_OPTIONS_INCOMPLETE,
	SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_UDP_HEADER_INCOMPLETE,
	SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_TCP_HEADER_INCOMPLETE,
	SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_UNHANDLED_PROTOCOL,
	SFE_IPV6_EXCEPTION_EVENT_ICMP_NO_CONNECTION,
	SFE_IPV6_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION,
	SFE_IPV6_EXCEPTION_EVENT_HEADER_INCOMPLETE,
	SFE_IPV6_EXCEPTION_EVENT_BAD_TOTAL_LENGTH,
	SFE_IPV6_EXCEPTION_EVENT_NON_V6,
	SFE_IPV6_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT,
	SFE_IPV6_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE,
	SFE_IPV6_EXCEPTION_EVENT_IP_OPTIONS_INCOMPLETE,
	SFE_IPV6_EXCEPTION_EVENT_UNHANDLED_PROTOCOL,
	SFE_IPV6_EXCEPTION_EVENT_FLOW_COOKIE_ADD_FAIL,
	SFE_IPV6_EXCEPTION_EVENT_LAST
};

/*
 * Per-module structure.
 */
struct sfe_ipv6 {
	spinlock_t lock;		/* Lock for SMP correctness */
	struct sfe_ipv6_connection_match *active_head;
					/* Head of the list of recently active connections */
	struct sfe_ipv6_connection_match *active_tail;
					/* Tail of the list of recently active connections */
	struct sfe_ipv6_connection *all_connections_head;
					/* Head of the list of all connections */
	struct sfe_ipv6_connection *all_connections_tail;
					/* Tail of the list of all connections */
	unsigned int num_connections;	/* Number of connections */
	struct timer_list timer;	/* Timer used for periodic sync ops */
	sfe_sync_rule_callback_t __rcu sync_rule_callback;
					/* Callback function registered by a connection manager for stats syncing */
	struct sfe_ipv6_connection *conn_hash[SFE_IPV6_CONNECTION_HASH_SIZE];
					/* Connection hash table */
	struct sfe_ipv6_connection_match *conn_match_hash[SFE_IPV6_CONNECTION_HASH_SIZE];
					/* Connection match hash table */
#ifdef CONFIG_NF_FLOW_COOKIE
	struct sfe_ipv6_flow_cookie_entry sfe_flow_cookie_table[SFE_FLOW_COOKIE_SIZE];
					/* flow cookie table*/
	sfe_ipv6_flow_cookie_set_func_t flow_cookie_set_func;
					/* function used to configure flow cookie in hardware*/
	int flow_cookie_enable;
					/* Enable/disable flow cookie at runtime */
#endif

	/*
	 * Stats recorded in a sync period. These stats will be added to
	 * connection_xxx64 after a sync period.
	 */
	u32 connection_create_requests;
					/* Number of IPv6 connection create requests */
	u32 connection_create_collisions;
					/* Number of IPv6 connection create requests that collided with existing hash table entries */
	u32 connection_destroy_requests;
					/* Number of IPv6 connection destroy requests */
	u32 connection_destroy_misses;
					/* Number of IPv6 connection destroy requests that missed our hash table */
	u32 connection_match_hash_hits;
					/* Number of IPv6 connection match hash hits */
	u32 connection_match_hash_reorders;
					/* Number of IPv6 connection match hash reorders */
	u32 connection_flushes;		/* Number of IPv6 connection flushes */
	u32 packets_forwarded;		/* Number of IPv6 packets forwarded */
	u32 packets_not_forwarded;	/* Number of IPv6 packets not forwarded */
	u32 exception_events[SFE_IPV6_EXCEPTION_EVENT_LAST];

	/*
	 * Summary statistics.
	 */
	u64 connection_create_requests64;
					/* Number of IPv6 connection create requests */
	u64 connection_create_collisions64;
					/* Number of IPv6 connection create requests that collided with existing hash table entries */
	u64 connection_destroy_requests64;
					/* Number of IPv6 connection destroy requests */
	u64 connection_destroy_misses64;
					/* Number of IPv6 connection destroy requests that missed our hash table */
	u64 connection_match_hash_hits64;
					/* Number of IPv6 connection match hash hits */
	u64 connection_match_hash_reorders64;
					/* Number of IPv6 connection match hash reorders */
	u64 connection_flushes64;	/* Number of IPv6 connection flushes */
	u64 packets_forwarded64;	/* Number of IPv6 packets forwarded */
	u64 packets_not_forwarded64;
					/* Number of IPv6 packets not forwarded */
	u64 exception_events64[SFE_IPV6_EXCEPTION_EVENT_LAST];

	/*
	 * Control state.
	 */
	struct kobject *sys_sfe_ipv6;	/* sysfs linkage */
	int debug_dev;			/* Major number of the debug char device */
	u32 debug_read_seq;		/* sequence number for debug dump */
};

/*
 * Enumeration of the XML output.
 */
enum sfe_ipv6_debug_xml_states {
	SFE_IPV6_DEBUG_XML_STATE_START,
	SFE_IPV6_DEBUG_XML_STATE_CONNECTIONS_START,
	SFE_IPV6_DEBUG_XML_STATE_CONNECTIONS_CONNECTION,
	SFE_IPV6_DEBUG_XML_STATE_CONNECTIONS_END,
	SFE_IPV6_DEBUG_XML_STATE_EXCEPTIONS_START,
	SFE_IPV6_DEBUG_XML_STATE_EXCEPTIONS_EXCEPTION,
	SFE_IPV6_DEBUG_XML_STATE_EXCEPTIONS_END,
	SFE_IPV6_DEBUG_XML_STATE_STATS,
	SFE_IPV6_DEBUG_XML_STATE_END,
	SFE_IPV6_DEBUG_XML_STATE_DONE
};

/*
 * XML write state.
 */
struct sfe_ipv6_debug_xml_write_state {
	enum sfe_ipv6_debug_xml_states state;
					/* XML output file state machine state */
	int iter_exception;		/* Next exception iterator */
};

typedef bool (*sfe_ipv6_debug_xml_write_method_t)(struct sfe_ipv6 *si, char *buffer, char *msg, size_t *length,
						  int *total_read, struct sfe_ipv6_debug_xml_write_state *ws);

void sfe_ipv6_exit(void);
int sfe_ipv6_init(void);

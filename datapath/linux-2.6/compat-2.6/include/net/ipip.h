#ifndef __NET_IPIP_WRAPPER_H
#define __NET_IPIP_WRAPPER_H 1

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)

#include <linux/if_tunnel.h>
#include <net/ip.h>

/* Keep error state on tunnel for 30 sec */
#define IPTUNNEL_ERR_TIMEO	(30*HZ)

struct ip_tunnel
{
	struct ip_tunnel	*next;
	struct net_device	*dev;
#ifndef HAVE_NETDEV_STATS
	struct net_device_stats stat;
#endif

	int			err_count;	/* Number of arrived ICMP errors */
	unsigned long		err_time;	/* Time when the last ICMP error arrived */

	/* These four fields used only by GRE */
	__u32			i_seqno;	/* The last seen seqno	*/
	__u32			o_seqno;	/* The last output seqno */
	int			hlen;		/* Precalculated GRE header length */
	int			mlink;

	struct ip_tunnel_parm	parms;

	struct ip_tunnel_prl_entry	*prl;		/* potential router list */
	unsigned int			prl_count;	/* # of entries in PRL */
};

/* ISATAP: default interval between RS in secondy */
#define IPTUNNEL_RS_DEFAULT_DELAY	(900)

struct ip_tunnel_prl_entry
{
	struct ip_tunnel_prl_entry	*next;
	__be32				addr;
	u16				flags;
	unsigned long			rs_delay;
	struct timer_list		rs_timer;
	struct ip_tunnel		*tunnel;
	spinlock_t			lock;
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25)
#define IPTUNNEL_XMIT() do {						\
	int err;							\
	int pkt_len = skb->len - skb_transport_offset(skb);		\
									\
	skb->ip_summed = CHECKSUM_NONE;					\
	iph->tot_len = htons(skb->len);                                 \
	ip_select_ident(iph, &rt->u.dst, NULL);				\
	ip_send_check(iph);                                             \
									\
	err = NF_HOOK(PF_INET, NF_IP_LOCAL_OUT, skb, NULL, rt->u.dst.dev, dst_output);\
	if (err == NET_XMIT_SUCCESS || err == NET_XMIT_CN) {            \
		stats->tx_bytes += pkt_len;				\
		stats->tx_packets++;					\
	} else {							\
		stats->tx_errors++;					\
		stats->tx_aborted_errors++;				\
	}								\
} while (0)
#else
#define IPTUNNEL_XMIT() do {						\
	int err;							\
	int pkt_len = skb->len;						\
									\
	skb->ip_summed = CHECKSUM_NONE;					\
	ip_select_ident(iph, &rt->u.dst, NULL);				\
									\
	err = ip_local_out(skb);					\
	if (net_xmit_eval(err) == 0) {					\
		stats->tx_bytes += pkt_len;				\
		stats->tx_packets++;					\
	} else {							\
		stats->tx_errors++;					\
		stats->tx_aborted_errors++;				\
	}								\
} while (0)
#endif

#else
#include_next <net/ipip.h>
#endif /* kernel < 2.6.32 */

#endif /* net/ipip.h wrapper */

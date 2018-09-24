/*
MIT License

Copyright(c) 2016 Trend Micro Incorporated

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
/*

TODO:

1- Check what kind of changes do to the packets.
   - Level 1 -> Modified. MAC
   - Level 2 -> Modified. IP
   - Level 3 -> Not modified in the same machine.

2- Implement the changes.

+ change MACs using the interfaces
- fix the case when two IPs are in the same list.
- Change IPs
- Change application layer?

*/

#include <iostream>
#include <string>
#include <set>
#include <map>

#include "pcap.h"

#include <protocols/layer2/common.hpp>
#include <protocols/layer2/ethernet2.hpp>
#include <protocols/layer3/ipv4.hpp>
#include <protocols/layer3/ethernet_ipv4_fragmenter.hpp>
#include <protocols/layer3/common.hpp>
#include <protocols/layer4/tcp.hpp>
#include <protocols/layer4/udp.hpp>
#include <windows/windows.hpp>

namespace replay {

	struct pcap_stat_t
	{
		pcap_stat_t()
		{
			count = 0;
			end_time.tv_sec = 0;
			end_time.tv_usec = 0;
			start_time.tv_sec = 0;
			start_time.tv_usec = 0;
		}

		timeval start_time;
		timeval end_time;
		unsigned long long	count;
	};

	struct pcap_devs_action_dump_t
	{
		pcap_devs_action_dump_t(std::ostream&  strm):m_strm(strm), m_count(0){}

		bool do_action(pcap_if_t* ifdev = 0)
		{
			if(ifdev == 0)
				return false;

			m_strm << m_count << "-Interface:" << std::endl;
			m_count++;

			if(ifdev->name != 0)
				m_strm << "    name:" << ifdev->name << std::endl;

			if(ifdev->description != 0)
				m_strm << "    description:" << ifdev->description << std::endl;

			uint64_t mac = 0;
			if(windows::netif::get_network_adapter(ifdev->name, mac))
				m_strm << "    mac:" << std::hex << mac << std::endl;

			if(ifdev->addresses != 0)
			{
				for (pcap_addr* address = ifdev->addresses; address != 0; address = address->next)
				{
					if(address->addr == 0)
						continue;

					if(address->addr->sa_data == 0)
						continue;

					//ip
					if(address->addr->sa_family == AF_INET)
					{
						m_strm << "    ip:" << inet_ntoa(((struct sockaddr_in*)address->addr)->sin_addr) << std::endl;
						continue;
					}					
				}
				
			}

			return false;					
		}

		std::ostream& m_strm;
		uint32_t m_count;
	};

	struct pcap_devs_action_if_by_name_t
	{
		pcap_devs_action_if_by_name_t(const char* ifname = 0)
		{
			if(ifname != 0)
				m_ifname.append(ifname);

			m_ifdev = 0;
		}

		pcap_if_t* get_if(){return m_ifdev;}

		bool do_action(pcap_if_t* ifdev = 0)
		{
			if(ifdev == 0)
				return false;

			if(ifdev->name != 0)
			{
				if(std::string(ifdev->name) == m_ifname)
				{
					m_ifdev = ifdev;
					return true;
				}
			}

			return false;					
		}
		
		pcap_if_t* m_ifdev;
		std::string m_ifname;
	};	

	struct pcap_devs_t
	{
		static bool get_first_ip_by_ifdef(uint32_t& ip, pcap_if_t* ifdev = 0)
		{
			if(ifdev == 0)
				return false;

			if(ifdev->addresses == 0)
				return false;

			for (pcap_addr* address = ifdev->addresses; address != 0; address = address->next)
			{
				if(address->addr == 0)
					continue;

				if(address->addr->sa_data == 0)
					continue;

				//ip
				if(address->addr->sa_family == AF_INET)
				{
					ip = ((struct sockaddr_in*)address->addr)->sin_addr.s_addr;
					return true;
				}					
			}

			return false;					
		}

		static bool get_first_ip_by_ifname(uint32_t& ip, const char* ifname = 0)
		{
			if(ifname == 0)
				return false;

			pcap_devs_t devs;
			pcap_devs_action_if_by_name_t if_by_name(ifname);

			devs.get_ifs(if_by_name);

			pcap_if_t* ifdev = if_by_name.get_if();

			if(ifdev == 0)
				return false;

			return get_first_ip_by_ifdef(ip, ifdev);
		}

		pcap_devs_t(): m_devs(0)
		{
			char errbuf[PCAP_ERRBUF_SIZE];
			pcap_findalldevs(&m_devs, errbuf);
		}

		~pcap_devs_t(){pcap_freealldevs(m_devs);}

		template<class Taction>
		void get_ifs(Taction& action)
		{
			if(m_devs == 0)
				return;

			while (m_devs != 0) 
			{
				if(action.do_action(m_devs))
					break;

				m_devs = m_devs->next;
			}	
		}
		
		pcap_if_t* m_devs;
	};

	struct offline_pcap_t
	{
		static bool open(offline_pcap_t& pcap, const char * file_path = 0)
		{
			if(file_path == 0)
				return false;

			char errbuf[PCAP_ERRBUF_SIZE];
			pcap.m_offline = pcap_open_offline(file_path, errbuf);	
			return pcap.m_offline != 0;
		}

		offline_pcap_t(): m_offline(0){}
		~offline_pcap_t()
		{ 
			if(m_offline != 0) 
				pcap_close(m_offline);
		}

		pcap_stat_t dump_stats()
		{
			if(m_offline == 0)
				return pcap_stat_t();

			pcap_stat_t pcap_stat;
			pcap_pkthdr pk_header;
			pk_header.ts.tv_sec = 0;
			pk_header.ts.tv_usec = 0;
			const u_char *data = 0;

			// get first packet
			if (pcap_next(m_offline, &pk_header))
			{
				pcap_stat.count++;
				pcap_stat.start_time = pk_header.ts;
			}

			while ((data = pcap_next(m_offline, &pk_header))) 
				pcap_stat.count++;

			pcap_stat.end_time = pk_header.ts;

			return pcap_stat;		
		}

		template<class Taction>
		void get_packets(Taction& action) const
		{
			if(m_offline == 0)
				return;
			
			pcap_pkthdr pk_header;
			const u_char *data = 0;
			layer2::types_t::enums dt = static_cast<layer2::types_t::enums>(pcap_datalink(m_offline));

			while (data = pcap_next(m_offline, &pk_header))
			{ 
				if(action.do_action(pk_header, data, dt))
					break;
			}		
		}
		
		pcap_t* m_offline;
	};


	class pcap_mirror_replay_t
	{
	public:
		static void play_back(pcap_mirror_replay_t& pcaplive, const offline_pcap_t& pcap){pcap.get_packets(pcaplive);}

	public:
		pcap_mirror_replay_t():m_livecap(0), m_failed_ptk_count(0){}

		~pcap_mirror_replay_t(){pcap_close(m_livecap);}

		
		/*
		   snaplen: maximum number of bytes to be captured by pcap
		   promisc: brings the interface into promiscuous mode
		   to_ms:   The read time out in milliseconds. A value of 0 means no time 
					out; on at least some platforms, this means that you may wait until 
					a sufficient number of packets arrive before seeing any packets, so you 
					should use a non-zero timeout).
		*/
		bool init(const char* ifname, int snaplen = 9999, bool promisc = true, int to_ms = 5)
		{
			if(ifname == 0)
				return false;

			char errbuf[PCAP_ERRBUF_SIZE];
			m_livecap = pcap_open_live(ifname, snaplen, promisc?1:0, to_ms, errbuf);
			
			if (m_livecap == 0) 
				return false;
			
			if (pcap_setnonblock(m_livecap, 1, errbuf) < 0) 
				return false;

			return true;
		}

		//send packets action.
		//return false if stop any further action by the caller.
		bool do_action(pcap_pkthdr& pk_header, const u_char *data, layer2::types_t::enums& data_link_type)
		{
			if(data == 0)
				return false;

			if(data_link_type == layer2::types_t::ethernet_10mb){
				layer2::ethernet2_header_t* mac = static_cast<layer2::ethernet2_header_t*>((void*)data);
	
			if(pcap_sendpacket(m_livecap, data, pk_header.caplen) < 0)
				m_failed_ptk_count++;
			}

			return false;
		}

		unsigned long long get_failed_packet_count(){return m_failed_ptk_count;}

	private:
		pcap_t* m_livecap;
		unsigned long long m_failed_ptk_count;
	};	

	struct host_t 
	{
		pcap_t* rif;
		uint64_t mac;
		uint32_t ip;
	};

	class pcap_layer2_split_replay_t
	{
	public:
		const int on_close_packet_delay = 0;
		const int on_packet_delay        = 0;
		const int on_packet_synack_delay = 0;

		const int on_n_packet_delay     = 50;
		const int on_n_packet           = 10;

	public:
		typedef std::set<uint64_t> mac_set_t;
		typedef std::map<uint64_t, host_t> host_map_t;

	public:
		static void play_back(pcap_layer2_split_replay_t& pcaplive, const offline_pcap_t& pcap){ pcap.get_packets(pcaplive); }

	public:
		pcap_layer2_split_replay_t():
			m_src_if(0), 
			m_dst_if(0), 
			m_pkt_count(0),
			m_pkt_replayed(0),
			m_failed_ptk_count(0),
		    m_l2_non_sp_ptk(0),
			m_last_count(0),
			m_disable_fragmentation(false){}

		~pcap_layer2_split_replay_t()
		{ 
			if(m_src_if != 0)
				pcap_close(m_src_if); 

			if (m_dst_if != 0)
				pcap_close(m_dst_if); 
		}
		
		/*
		   snaplen: maximum number of bytes to be captured by pcap
		   promisc: brings the interface into promiscuous mode
		   to_ms:   The read time out in milliseconds. A value of 0 means no time 
					out; on at least some platforms, this means that you may wait until 
					a sufficient number of packets arrive before seeing any packets, so you 
					should use a non-zero timeout).
		*/
		bool init(const char* src_ifname, const char* dst_ifname, bool disable_frag = false, 
			int snaplen = 9999, bool promisc = true, int to_ms = 5)
		{
			m_src_if = 0;
			m_dst_if = 0;
			m_pkt_count = 0;
			m_failed_ptk_count = 0;
			m_l2_non_sp_ptk = 0;
			m_disable_fragmentation = disable_frag;

			if(!open_if(src_ifname, &m_src_if, snaplen, promisc, to_ms))
				return false;

			if(!open_if(dst_ifname, &m_dst_if, snaplen, promisc, to_ms))
				return false;

			if(!windows::netif::get_network_adapter(src_ifname, m_src_if_mac))
				return false;

			if(!windows::netif::get_network_adapter(dst_ifname, m_dst_if_mac))
				return false;

			if(!pcap_devs_t::get_first_ip_by_ifname(m_src_if_ip, src_ifname))
				return false;

			return pcap_devs_t::get_first_ip_by_ifname(m_dst_if_ip, dst_ifname);
		}

		bool init(const char* src_ifname, 
			const char* dst_ifname, 
			const char* src_ip, 
			const char* dst_ip, 
			bool disable_frag = false,
			int snaplen = 9999, 
			bool promisc = true, 
			int to_ms = 5)
		{
			m_src_if = 0;
			m_dst_if = 0;
			m_pkt_count = 0;
			m_failed_ptk_count = 0;
			m_l2_non_sp_ptk = 0;
			m_disable_fragmentation = disable_frag;

			if (!open_if(src_ifname, &m_src_if, snaplen, promisc, to_ms))
				return false;

			if (!open_if(dst_ifname, &m_dst_if, snaplen, promisc, to_ms))
				return false;

			if (!windows::netif::get_network_adapter(src_ifname, m_src_if_mac))
				return false;

			if (!windows::netif::get_network_adapter(dst_ifname, m_dst_if_mac))
				return false;

			m_src_if_ip = inet_addr(src_ip);
			if (m_src_if_ip == INADDR_NONE)
				return false;
			
			m_dst_if_ip = inet_addr(dst_ip);
			
			return (m_dst_if_ip != INADDR_NONE);
		}

		/*
			send packets action.
			return false if stop any further action by the caller.
			
			note:
			 we are doing the best effort to replay the pcap.because of that some fixes to the packet are
			 done. example of fixes:
				- packets can have Ethernet padding bytes at the end. pcap lib will fail to replay them if
				  the total size is greater than the mtu. Here we are calculating the "util" pcayload to replay.
				
				- somes time IP packet length is greater than pcap packet size. this happens when:
				  "Packet size limited during capture"

				- fix pcap IP packet length: some times "packet_length" is 0 becuase "TCP segmentation offload". 
				  We try to fix it here acording to packet data size.
		*/
		
		bool do_action(pcap_pkthdr& pk_header, const u_char *data, layer2::types_t::enums& data_link_type)
		{
			if(data == 0)
				return false;

			m_pkt_count++;
			windows::system::sleep(on_packet_delay);

			if(data_link_type == layer2::types_t::ethernet_10mb)
			{
				layer2::ethernet2_header_t* eth_header = static_cast<layer2::ethernet2_header_t*>((void*)data);
				
				if(eth_header->get_payload_type() != layer3::types_t::ipv4)
					return false;

			    layer3::ipv4::ethernet_ipv4_header_t* eth_ip_header = 
					layer3::ipv4::ethernet_ipv4_header_t::get_header(data);

				//fix pcap IP packet length.
				//some times "packet_length" is 0 becuase "TCP segmentation offload". We try to fix it here acording to
				//packet data size.
				if(eth_ip_header->ip_header.packet_length == 0)
					eth_ip_header->ip_header.packet_length = pk_header.caplen - eth_ip_header->eth_header.length;

				//somes time IP packet length is greater than pcap packet size.
				//this happens when "Packet size limited during capture"
				if (eth_ip_header->ip_header.packet_length > pk_header.caplen - eth_ip_header->eth_header.length)
					eth_ip_header->ip_header.packet_length = pk_header.caplen - eth_ip_header->eth_header.length;

				//pcaps can have Ethernet padding bytes at the end. pcap lib will fail to replay them if
				//the total size is greater than the mtu. Here we are calculating the "util" pcayload to replay.
				uint16_t full_packet_len = eth_ip_header->eth_header.length + eth_ip_header->ip_header.packet_length;
								
				if (eth_ip_header->ip_header.upper_layer_type() == layer4::types_t::enums::tcp) 
				{
					layer4::tcp::eth_ipv4_tcp_header_t hd;
					if (!layer4::tcp::eth_ipv4_tcp_header_t::get_eth_ipv4_tcp_header(data, &hd)) {
						m_failed_ptk_count++;
						return false;
					}			

					if(hd.is_syn_ack())
						windows::system::sleep(on_packet_synack_delay);
					
					uint64_t sh = hd.src_host_hash();
					uint64_t dh = hd.dst_host_hash();

					if (m_host_tcp_map.find(sh) == m_host_tcp_map.end()) {
						m_host_tcp_map[sh].rif = m_src_if;
						m_host_tcp_map[sh].mac = m_src_if_mac;
						m_host_tcp_map[sh].ip  = m_src_if_ip;
					}

					if (m_host_tcp_map.find(dh) == m_host_tcp_map.end()) {
						m_host_tcp_map[dh].rif = m_dst_if;
						m_host_tcp_map[dh].mac = m_dst_if_mac;
						m_host_tcp_map[dh].ip = m_dst_if_ip;
					}

					eth_header->update_src(m_host_tcp_map[sh].mac);
					eth_header->update_dst(m_host_tcp_map[dh].mac);

					eth_ip_header->ip_header.change_ips(m_host_tcp_map[sh].ip, m_host_tcp_map[dh].ip);

					if (layer3::frag::ethernet_ipv4_fragmenter::fragment(data, pk_header.caplen))
					{
						//Fragemented packet should keep the same TCP checksum as the original
						//non fragmented packet.
						fix_checksum(eth_ip_header, data);
						uint8_t frags = layer3::frag::ethernet_ipv4_fragmenter::do_fragment(m_host_tcp_map[sh].rif, 
							data, pk_header.caplen, 0, m_disable_fragmentation);

						if (frags == 0)
							m_failed_ptk_count++;
						else
							m_pkt_replayed += frags;
					}
					else {

						fix_checksum(eth_ip_header, data);
						if (pcap_sendpacket(m_host_tcp_map[sh].rif, data, full_packet_len) < 0)
							m_failed_ptk_count++;
						else
							m_pkt_replayed++;
					}
				}
				else if (eth_ip_header->ip_header.upper_layer_type() == layer4::types_t::enums::udp)
				{
					layer4::udp::eth_ipv4_udp_packet_t hd;
					if (!layer4::udp::eth_ipv4_udp_packet_t::parse(data, &hd)){
						m_failed_ptk_count++;
						return false;
					}

					uint64_t sh = hd.src_host_hash();
					uint64_t dh = hd.dst_host_hash();

					if (m_host_udp_map.find(sh) == m_host_udp_map.end()) {
						m_host_udp_map[sh].rif = m_src_if;
						m_host_udp_map[sh].mac = m_src_if_mac;
						m_host_udp_map[sh].ip = m_src_if_ip;
					}

					if (m_host_udp_map.find(dh) == m_host_udp_map.end()) {
						m_host_udp_map[dh].rif = m_dst_if;
						m_host_udp_map[dh].mac = m_dst_if_mac;
						m_host_udp_map[dh].ip = m_dst_if_ip;
					}

					eth_ip_header->ip_header.change_ips(m_host_udp_map[sh].ip, m_host_udp_map[dh].ip);

					if (layer3::frag::ethernet_ipv4_fragmenter::fragment(data, pk_header.caplen))
					{
						//Fragemented packet should keep the same TCP checksum as the original
						//non fragmented packet.
						fix_checksum(eth_ip_header, data);
						uint8_t frags = layer3::frag::ethernet_ipv4_fragmenter::do_fragment(m_host_udp_map[sh].rif, 
							data, pk_header.caplen, 0, m_disable_fragmentation);

						if (frags == 0)
							m_failed_ptk_count++;
						else
							m_pkt_replayed += frags;
					}
					else {

						fix_checksum(eth_ip_header, data);
						if (pcap_sendpacket(m_host_udp_map[sh].rif, data, full_packet_len) < 0)
							m_failed_ptk_count++;
						else
							m_pkt_replayed++;
					}
				}
				else //non udp or tcp
				{
					uint64_t sh = eth_ip_header->src_host_hash();
					uint64_t dh = eth_ip_header->dst_host_hash();

					if (m_host_other_map.find(sh) == m_host_other_map.end()) {
						m_host_other_map[sh].rif = m_src_if;
						m_host_other_map[sh].mac = m_src_if_mac;
						m_host_other_map[sh].ip = m_src_if_ip;
					}

					if (m_host_other_map.find(dh) == m_host_other_map.end()) {
						m_host_other_map[dh].rif = m_dst_if;
						m_host_other_map[dh].mac = m_dst_if_mac;
						m_host_other_map[dh].ip = m_dst_if_ip;
					}

					eth_ip_header->ip_header.change_ips(m_host_other_map[sh].ip, m_host_other_map[dh].ip);

					if (layer3::frag::ethernet_ipv4_fragmenter::fragment(data, pk_header.caplen))
					{
						//Fragemented packet should keep the same TCP checksum as the original
						//non fragmented packet.
						fix_checksum(eth_ip_header, data);
						uint8_t frags = layer3::frag::ethernet_ipv4_fragmenter::do_fragment(m_host_other_map[sh].rif,
							data, pk_header.caplen, 0, m_disable_fragmentation);

						if (frags == 0)
							m_failed_ptk_count++;
						else
							m_pkt_replayed += frags;
					}
					else {

						fix_checksum(eth_ip_header, data);
						if (pcap_sendpacket(m_host_other_map[sh].rif, data, full_packet_len) < 0)
							m_failed_ptk_count++;
						else
							m_pkt_replayed++;
					}
				}
			}
			else
				m_l2_non_sp_ptk++;
			
			return false;
		}

		bool fix_checksum(layer3::ipv4::ethernet_ipv4_header_t* ip, const u_char *data)
		{
			if (ip->ip_header.upper_layer_type() == layer4::types_t::enums::tcp)
				return layer4::tcp::eth_ipv4_tcp_header_t::fix_checksum(data);
			
			if (ip->ip_header.upper_layer_type() == layer4::types_t::enums::udp)
				return layer4::udp::eth_ipv4_udp_packet_t::fix_checksum(data);

			return false;
		}

		void clean_stats() 
		{
			m_pkt_count = 0;
			m_failed_ptk_count = 0;
			m_l2_non_sp_ptk = 0;
			m_pkt_replayed = 0;
		}

		bool has_bad_ptks() { return m_failed_ptk_count > 0 || m_l2_non_sp_ptk > 0; }

		uint64_t get_replayed_packet_count() { return m_pkt_replayed; }

		uint64_t get_packet_count() { return m_pkt_count; }

		uint64_t get_failed_packet_count(){return m_failed_ptk_count;}

		uint64_t get_l2_non_supported_packet_count() { return m_l2_non_sp_ptk; }

	private:
		bool open_if(const char* dst_ifname, pcap_t** ifhandle, int snaplen = 9999, bool promisc = true, int to_ms = 5)
		{
			if(dst_ifname == 0 || dst_ifname == 0 || ifhandle == 0)
				return false;

			char errbuf[PCAP_ERRBUF_SIZE];
			*ifhandle = pcap_open_live(dst_ifname, snaplen, promisc?1:0, to_ms, errbuf);

			if (*ifhandle == 0) 
				return false;

			if (pcap_setnonblock(*ifhandle, 1, errbuf) < 0) 
				return false;

			return true;
		}

	private:
		pcap_t* m_src_if;
		uint64_t m_src_if_mac;
		uint32_t m_src_if_ip;
		uint64_t m_dst_if_mac;
		uint32_t m_dst_if_ip;
		pcap_t* m_dst_if;
		mac_set_t m_src_set;
		mac_set_t m_dst_set;

		host_map_t m_host_tcp_map;
		host_map_t m_host_udp_map;
		host_map_t m_host_other_map;

		//when fragementation is needed 
		//olly the first packet is sent.
		bool m_disable_fragmentation; 

		uint64_t m_pkt_count;
		uint64_t m_pkt_replayed;
		uint64_t m_failed_ptk_count;
		uint64_t m_l2_non_sp_ptk;

		uint64_t m_last_count;

	};
}
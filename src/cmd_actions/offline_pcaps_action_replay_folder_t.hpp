#pragma once

#include "traffic_replay.hpp"
#include <cmd_actions/pcap_devs_action_getif_t.hpp>

namespace replay {
	
	struct offline_pcaps_action_replay_t
	{
	public:
		offline_pcaps_action_replay_t() :
			pcap_count(0),
			m_failed_packet_count(0),
			m_packet_count(0),
			m_replayed_packet_count(0),
			m_l2_non_supported_packet_count(0)
		{}

		bool init(const char* src, const char* dst, bool ip_if)
		{
			if (ip_if)
			{
				pcap_devs_t devs_1;
				pcap_devs_action_getif_t src_if(src);
				devs_1.get_ifs(src_if);

				pcap_devs_t devs_2;
				pcap_devs_action_getif_t dst_if(dst);
				devs_2.get_ifs(dst_if);

				return m_rsplit.init(src_if.m_id.c_str(), dst_if.m_id.c_str());
			}
			
			return m_rsplit.init(src, dst);
		}

		bool do_action(const char* pcap_file)
		{
			pcap_count++;
			
			offline_pcap_t pcap;
			if (!pcap.open(pcap, pcap_file)) 
			{
				corrupted_pcaps.push_back(pcap_file);
				return false;
			}

			pcap_layer2_split_replay_t::play_back(m_rsplit, pcap);

			m_replayed_packet_count += m_rsplit.get_replayed_packet_count();
			m_packet_count += m_rsplit.get_packet_count();

			if (m_rsplit.has_bad_ptks())
			{
				m_failed_packet_count += m_rsplit.get_failed_packet_count();
				m_l2_non_supported_packet_count += m_rsplit.get_l2_non_supported_packet_count();				
				failed_replays_pcaps.push_back(pcap_file);
			}

			m_rsplit.clean_stats();

			return true;
		}		

		void dump_stats() 
		{
			std::cout << "Pcap stats: " << std::endl;
			std::cout << "  Total pcaps:               " << pcap_count << std::endl;
			std::cout << "  Corrupted pcaps:           " << corrupted_pcaps.size() << std::endl;
			std::cout << "  Total packets:             " << m_packet_count << std::endl;
			std::cout << "  Replayed packets:          " << m_replayed_packet_count << std::endl;
			std::cout << "  Pcaps with failed packets: " << failed_replays_pcaps.size() << std::endl;
			std::cout << "  L2 non-suported packets:   " << m_l2_non_supported_packet_count << std::endl;
			std::cout << "  Failed packet count:       " << m_failed_packet_count << std::endl;		

			if (corrupted_pcaps.size() > 0)
			{
				std::cout << "  Corrupted pcaps list :" << std::endl;
				for (size_t i = 0; i < corrupted_pcaps.size(); i++)
					std::cout << "   " << corrupted_pcaps[i] << std::endl;
			}
			
			if (failed_replays_pcaps.size() > 0)
			{
				std::cout << "  Pcaps with failed packets list :" << std::endl;
				for (size_t i = 0; i < failed_replays_pcaps.size(); i++)
					std::cout << "    " << failed_replays_pcaps[i] << std::endl;
			}
		}

		pcap_layer2_split_replay_t m_rsplit;
		uint32_t pcap_count;
		std::string m_src_ifname;
		std::string m_dst_ifname;
		bool m_ip_if;
		std::vector<std::string> corrupted_pcaps;

		uint64_t m_failed_packet_count;
		uint64_t m_packet_count;
		uint64_t m_l2_non_supported_packet_count;
		uint64_t m_replayed_packet_count;
		std::vector<std::string> failed_replays_pcaps;
	};	
}
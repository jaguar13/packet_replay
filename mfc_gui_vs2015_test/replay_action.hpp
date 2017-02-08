#pragma once

#include "stdafx.h"
#include "OutputWnd.h"
#include "traffic_replay.hpp"
#include <cmd_actions/pcap_devs_action_getif_t.hpp>

namespace replay_gui {

	struct replay_action
	{
	public:
		COutputWnd*  m_wndOutput;

		uint64_t failed_packet_count = 0;
		uint64_t l2_non_supported_packet_count = 0;
		uint64_t replayed_packet_count = 0;
		uint64_t packet_count = 0;
		uint64_t pcap_count = 0;

		std::vector<std::string> corrupted_pcaps;
		std::vector<std::string> failed_replays_pcaps;
		std::vector<std::string> l2_non_supported_replays_pcaps;

		std::string src;
		std::string dst;

		replay::pcap_layer2_split_replay_t replay;

	public:	
		bool init_from_if(std::string src, std::string dst)
		{
			m_wndOutput->DebugText(CString(_T("Configuring interfaces...")));
			if (!replay.init(src.c_str(), dst.c_str())) {
				m_wndOutput->DebugText(CString(_T("Error configuring interfaces:")));
				return false;
			}
			else
				m_wndOutput->DebugText(CString(_T("Successfully configuring interfaces:")));

			if (src == dst)
				m_wndOutput->DebugText(CString(_T("Warnning!! Source and Destination interface are the same.")));

			m_wndOutput->DebugText(CString(_T("Source:      ")) + CString(src.c_str()));
			m_wndOutput->DebugText(CString(_T("Destination: ")) + CString(dst.c_str()));

			return true;
		}

		bool do_action(const char* pcap_file)
		{
			pcap_count++;
			
			m_wndOutput->DebugText(CString(_T("Replaying pcap: ")) + CString(pcap_file));

			replay::offline_pcap_t pcap;
			if (!pcap.open(pcap, pcap_file))
			{
				corrupted_pcaps.push_back(pcap_file);
				m_wndOutput->DebugText(CString(_T("Error corrupted pcap: ")) + CString(pcap_file));
				return false;
			}

			replay::pcap_layer2_split_replay_t::play_back(replay, pcap);

			replayed_packet_count += replay.get_replayed_packet_count();
			packet_count += replay.get_packet_count();

			if (replay.has_bad_ptks())
			{
				if (replay.get_l2_non_supported_packet_count() > 0)
					l2_non_supported_replays_pcaps.push_back(pcap_file);

				if (replay.get_failed_packet_count() > 0)
					failed_replays_pcaps.push_back(pcap_file);

				failed_packet_count += replay.get_failed_packet_count();
				l2_non_supported_packet_count += replay.get_l2_non_supported_packet_count();
			}

			return true;
		}

		void clean_stats() 
		{
			replay.clean_stats();
			failed_packet_count = 0;
			l2_non_supported_packet_count = 0;
			replayed_packet_count = 0;
			packet_count = 0;
			pcap_count = 0;

			corrupted_pcaps.clear();
			failed_replays_pcaps.clear();
			l2_non_supported_replays_pcaps.clear();
		}
		
		void dump_stats()
		{
			CString stat;
			m_wndOutput->DebugText(CString(_T("Replay stats: ")));

			stat.Format(_T("  Total pcaps: %d"), pcap_count);
			m_wndOutput->DebugText(stat);
			stat.Format(_T("  Corrupted pcaps: %d"), corrupted_pcaps.size());
			m_wndOutput->DebugText(stat);
			stat.Format(_T("  Total packets: %d"), packet_count);
			m_wndOutput->DebugText(stat);
			stat.Format(_T("  Replayed packets: %d"), replayed_packet_count);
			m_wndOutput->DebugText(stat);
			stat.Format(_T("  Pcaps with failed packets: %d"), failed_replays_pcaps.size());
			m_wndOutput->DebugText(stat);
			stat.Format(_T("  L2 non-suported packets: %d"), l2_non_supported_packet_count);
			m_wndOutput->DebugText(stat);
			stat.Format(_T("  Failed packet count: %d"), failed_packet_count);
			m_wndOutput->DebugText(stat);

			if (corrupted_pcaps.size() > 0)
			{
				m_wndOutput->DebugText(CString(_T("  Corrupted pcaps list :")));
				for (size_t i = 0; i < corrupted_pcaps.size(); i++)
					m_wndOutput->DebugText(CString(_T("   ")) + CString(corrupted_pcaps[i].c_str()));
			}

			if (l2_non_supported_replays_pcaps.size() > 0)
			{
				m_wndOutput->DebugText(CString(_T("  Pcaps with L2 layer non-suppored :")));
				for (size_t i = 0; i < l2_non_supported_replays_pcaps.size(); i++)
					m_wndOutput->DebugText(CString(_T("   ")) + CString(l2_non_supported_replays_pcaps[i].c_str()));
			}

			if (failed_replays_pcaps.size() > 0)
			{
				m_wndOutput->DebugText(CString(_T("  Pcaps with failed packets list :")));
				for (size_t i = 0; i < failed_replays_pcaps.size(); i++)
					m_wndOutput->DebugText(CString(_T("   ")) + CString(failed_replays_pcaps[i].c_str()));
			}

			if (corrupted_pcaps.size() > 0
				|| failed_replays_pcaps.size() > 0
				|| l2_non_supported_replays_pcaps.size() > 0)
				m_wndOutput->DebugText(CString(_T("Error replaying some pcaps..")));
			else
				m_wndOutput->DebugText(CString(_T("Successfully replay.")));
		}
	};
}
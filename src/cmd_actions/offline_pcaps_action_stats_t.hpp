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

#include "traffic_replay.hpp"

namespace replay {
	
	struct offline_pcaps_action_stats_t
	{
	public:
		offline_pcaps_action_stats_t(): pcap_count(0){}

		bool do_action(const char* pcap_file)
		{
			offline_pcap_t pcap;
			pcap_count++;
			if (!pcap.open(pcap, pcap_file)) 
			{
				std::cout << pcap_count << ": corrupted pcap: " << pcap_file << std::endl;
				corrupted_pcaps.push_back(pcap_file);
				return false;
			}

			pcap_stat_t sta = pcap.dump_stats();
			std::cout << pcap_count << ": Pcap stats: " << pcap_file  << std::endl;
			std::cout << "   start_time: " << sta.start_time.tv_sec << std::endl;
			std::cout << "   end_time:   " << sta.end_time.tv_sec << std::endl;
			std::cout << "   pkts:       " << sta.count << std::endl;

			return true;
		}

		void dump_stats() 
		{
			std::cout << "Pcap stats: " << pcap_file << std::endl;
			std::cout << "  total pcaps:     " << pcap_count << std::endl;
			std::cout << "  corrupted pcaps: " << corrupted_pcaps.size() << std::endl;

			for (size_t i = 0; i < corrupted_pcaps.size(); i++) 
				std::cout << "  " << corrupted_pcaps[i] << std::endl;
		}

		uint32_t pcap_count;
		std::vector<std::string> corrupted_pcaps;
	};	
}
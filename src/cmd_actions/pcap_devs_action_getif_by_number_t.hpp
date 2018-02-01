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

#include <string>
#include "traffic_replay.hpp"

namespace replay {

	struct pcap_devs_action_getif_by_number_t
	{
	public:
		pcap_devs_action_getif_by_number_t(uint8_t n) :m_n(n), m_count(0){}

		bool do_action(pcap_if_t* ifdev = 0)
		{
			if (ifdev == 0)
				return false;

			if (m_n == m_count) {
				m_id = ifdev->name;
				return true;
			}

			m_count++;
			return false;			
		}

		std::string m_id;
		uint8_t m_n;
		uint8_t m_count;
	};
}
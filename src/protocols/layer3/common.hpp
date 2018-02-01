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
#include "bittypes.h"
#include "pcap.h"

namespace layer3 {

	struct types_t {
		enum enums: uint16_t {
			pup                       = 0x0200,
			ipv4                      = 0x0800,
			arp                       = 0x0806,
			rarp                      = 0x8035,
			vlan_tagging_8021q        = 0x8100,
			eap_authentication_8021x  = 0x888e,
			mpls	                  = 0x8847,
			test_interfaces	          = 0x9000,
			ipv6	                  = 0x86DD
		};
	};	

}
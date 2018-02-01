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
#include "pcap.h"

namespace layer2 {
	struct types_t {
		enum enums: int {
			bsd_loopback        = DLT_NULL,        /* BSD loopback encapsulation */
			ethernet_10mb       = DLT_EN10MB,      /* Ethernet (10Mb) */
			ethernet_3mb        = DLT_EN3MB,       /* Experimental Ethernet (3Mb) */
			amateur_radio_ax25  = DLT_AX25,        /* Amateur Radio AX.25 */
			pronet	            = DLT_PRONET,      /* Proteon ProNET Token Ring */
			chaos	            = DLT_CHAOS,       /* Chaos */
			ieee802_token_ring	= DLT_IEEE802,     /* 802.5 Token Ring */
			arcnet	            = DLT_ARCNET,      /* ARCNET, with BSD-style header */
			serial_line_ip	    = DLT_SLIP,        /* Serial Line IP */
			ppp		            = DLT_PPP,         /* Point-to-point Protocol */
			fddi	            = DLT_FDDI,        /* FDDI */			
		};
	};

	struct mac_address
	{
		static uint64_t to_uint64_t(uint8_t* dst_mac)
		{
			if(dst_mac == 0)
				return 0;

			return	uint64_t(dst_mac[0]) << 40 |
				uint64_t(dst_mac[1]) << 32 |
				uint64_t(dst_mac[2]) << 24 |
				uint64_t(dst_mac[3]) << 16 |
				uint64_t(dst_mac[4]) << 8 |
				uint64_t(dst_mac[5]);

		}
	};
}
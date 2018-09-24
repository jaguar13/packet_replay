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
#include <stdint.h>
#include <endian/endianness.hpp>
#include <crypto/checksum.hpp>
#include <crypto/hash.hpp>

namespace layer4 {
	namespace tcp {	

		const int16_t FLAG_FINISH     = 0x0001;
		const int16_t FLAG_SYN        = 0x0002;		
		const int16_t FLAG_ACK        = 0x0010;
		const int16_t FLAG_SYN_ACK    = FLAG_SYN | FLAG_ACK;
		const int16_t FLAG_RESET      = 0x0004;

		#pragma pack(push)
		#pragma pack(1)
		struct tcp_header_t
		{
			endian::int16_bigendia_t port_src;
			endian::int16_bigendia_t port_dst;
			endian::int32_bigendia_t seq_no;
			endian::int32_bigendia_t ack_no;
			endian::int16_bigendia_t flags;
			endian::int16_bigendia_t window;
			endian::int16_bigendia_t check_sum;
			endian::int16_bigendia_t urgent;

			static tcp_header_t* get_header(const u_char* data, uint16_t offset)
			{
				return static_cast<tcp_header_t*>((void*)&data[offset]);
			}

			bool is_reset() { return (flags &  FLAG_RESET) == FLAG_RESET; }

			bool is_finish() { return (flags &  FLAG_FINISH) == FLAG_FINISH; }

			bool is_syn_ack() { return (flags &  FLAG_SYN_ACK) == FLAG_SYN_ACK; }

			bool is_close_connection() { return is_finish() || is_reset(); }
		}; 
		#pragma pack(pop)

		struct eth_ipv4_tcp_header_t
		{
			layer3::ipv4::ethernet_ipv4_header_t* eth_ipv4;
			tcp_header_t* tcp;

		public:
			static bool get_eth_ipv4_tcp_header(const u_char* data, eth_ipv4_tcp_header_t* hd)
			{
				layer2::ethernet2_header_t* eth_header = static_cast<layer2::ethernet2_header_t*>((void*)data);

				if (eth_header->get_payload_type() != layer3::types_t::ipv4)
					return false;

				hd->eth_ipv4 = layer3::ipv4::ethernet_ipv4_header_t::get_header(data);

				if (hd->eth_ipv4->ip_header.upper_layer_type() != layer4::types_t::enums::tcp)
					return false;

				hd->tcp = layer4::tcp::tcp_header_t::get_header(data, hd->eth_ipv4->get_abs_upper_offset());
				return true;
			}

			static bool fix_checksum(const u_char* data)
			{
				eth_ipv4_tcp_header_t hd;
				if (!get_eth_ipv4_tcp_header(data, &hd))
					return false;

				hd.tcp->check_sum = 0;

				uint32_t sum = crypto::checksum_t::sum16_bits(
					static_cast<uint8_t*>((void*)&(hd.eth_ipv4->ip_header.source_address)), 0, 8);

				uint16_t len = hd.eth_ipv4->ip_header.payload_length();

				sum += IPPROTO_TCP + len;
				sum += crypto::checksum_t::sum16_bits(
					static_cast<uint8_t*>((void*)&(data[hd.eth_ipv4->get_abs_upper_offset()])), 0, len);

				hd.tcp->check_sum = crypto::checksum_t::sum16bits_to_checksum(sum);

				return true;
			}			
		

		public:
			bool is_close_connection() { return tcp->is_close_connection(); }

			bool is_syn_ack() { return tcp->is_syn_ack(); }

			uint64_t src_host_hash()
			{
				uint64_t mac = eth_ipv4->eth_header.get_src();
				uint64_t ip = eth_ipv4->ip_header.source_address;
				uint64_t port = tcp->port_src;

				uint64_t hash = 0;
				
				crypto::hash::hash_t::quick_hash(&mac, sizeof(uint64_t), hash);
				hash = crypto::hash::hash_t::quick_hash(&ip, sizeof(uint64_t), hash);
				return crypto::hash::hash_t::quick_hash(&port, sizeof(uint64_t), hash);
			}

			uint64_t dst_host_hash()
			{
				uint64_t mac = eth_ipv4->eth_header.get_dst();
				uint64_t ip = eth_ipv4->ip_header.destination_address;
				uint64_t port = tcp->port_dst;

				uint64_t hash = 0;

				crypto::hash::hash_t::quick_hash(&mac, sizeof(uint64_t), hash);
				hash = crypto::hash::hash_t::quick_hash(&ip, sizeof(uint64_t), hash);
				return crypto::hash::hash_t::quick_hash(&port, sizeof(uint64_t), hash);
			}
		};
	}	
}
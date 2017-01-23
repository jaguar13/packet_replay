#pragma once
#include <bittypes.h>
#include <protocols/layer2/ethernet2.hpp>
#include <protocols/layer4/common.hpp>
#include <endian/endianness.hpp>
#include <crypto/checksum.hpp>

namespace layer3 {
	namespace ipv4 {

		#pragma pack(push)
		#pragma pack(1)
		struct ipv4_header_t
		{
			uint8_t version_ihl;
			uint8_t type_of_service;
			endian::uint16_bigendia_t packet_length;
			endian::uint16_bigendia_t identification;
			endian::uint16_bigendia_t fragment_offset;
			uint8_t time_to_live;
			uint8_t upper_protocol;
			endian::uint16_bigendia_t header_checksum;
			endian::uint32_bigendia_t source_address;
			endian::uint32_bigendia_t destination_address;

		public:
			static uint32_t to_rev_uint32_t(uint32_t ip)
			{
				uint8_t* ip_ptr = static_cast<uint8_t*>((void*)&ip);

				return uint32_t(ip_ptr[0]) << 24 |
					uint32_t(ip_ptr[1]) << 16 |
					uint32_t(ip_ptr[2]) << 8 |
					uint32_t(ip_ptr[3]);
			}

			static uint16_t to_rev_uint16_t(uint16_t val)
			{
				uint8_t* val_ptr = static_cast<uint8_t*>((void*)&val);

				return uint32_t(val_ptr[0]) << 8 |
					uint32_t(val_ptr[1]);
			}

			static ipv4_header_t* parse(const u_char* data, uint16_t offset = 0)
			{
				return static_cast<ipv4_header_t*>((void*)&data[offset]);
			}

		public:
			void change_ips(uint32_t src_ip, uint32_t dst_ip)
			{
				source_address = to_rev_uint32_t(src_ip); 
				destination_address = to_rev_uint32_t(dst_ip);
				fix_header_checksum();
			}

			uint16_t get_length(){return (version_ihl & 0x0F) * 4;}

			layer4::types_t::enums upper_layer_type() { return static_cast<layer4::types_t::enums>(upper_protocol); }

			uint16_t payload_length() { return packet_length - get_length(); }

			void clear_more_fragments() { fragment_offset = fragment_offset & 0x1FFF; }

			void set_fragment_offset(uint16_t offset) {	fragment_offset = (offset >> 3) | 0x2000; }

		private:
			void fix_header_checksum()
			{
				header_checksum = 0;

				//TODO: support ip options in the header. The checksum require the 
				//      total length of the header from get_length().
				if(sizeof(ipv4_header_t) == get_length()) //no ip options in the header.
					header_checksum = to_rev_uint16_t(crypto::checksum_t::calc(
						static_cast<uint16_t*>((void*)this), sizeof(ipv4_header_t)));
			}
		};

		#pragma pack(pop)
		
		#pragma pack(push)
		#pragma pack(1)
		struct ethernet_ipv4_header_t
		{
			layer2::ethernet2_header_t eth_header;
			ipv4_header_t ip_header;

			//TODO: Add support for IP Header Options.

		public:
			static ethernet_ipv4_header_t* get_header(const u_char* data, uint16_t offset = 0)
			{
				return static_cast<ethernet_ipv4_header_t*>((void*)&data[offset]);
			}

		public:
			uint16_t get_abs_upper_offset() { return ip_header.get_length() + layer2::ethernet2_header_t::length; }	

			uint16_t header_length() { return get_abs_upper_offset(); }
		};
		#pragma pack(pop)


		/*
			Maximum Transmission Unit (MTU) is the maximum length of data that can be 
			transmitted by a protocol in one instance. If we take the Ethernet interface 
			as an example, the MTU size of an Ethernet interface is 1500 bytes by default, 
			which excludes the Ethernet frame header and trailer. It means that the interface 
			cannot carry any frame larger then 1500 bytes. If we look inside the frame, 
			we have a 20 byte IP header + 20 byte TCP header, leaving a 1460 byte of the 
			payload that can be transmitted in one frame.
		*/
		struct ethernet_ipv4_fragmenter
		{
			static const uint16_t mtu_default = 1500;			

		public:
			static bool fragment(const u_char* data, uint32_t data_length, uint16_t offset = 0) 
			{
				ethernet_ipv4_header_t* ethip = ethernet_ipv4_header_t::get_header(data, offset);
				return ethip->ip_header.packet_length > mtu_default;
			}
			
			static uint8_t do_fragment(pcap_t* dst_if, const u_char* data, uint32_t data_length, uint16_t offset = 0)
			{
				ethernet_ipv4_header_t* ethip = ethernet_ipv4_header_t::get_header(data, offset);

				if (ethip->ip_header.packet_length <= mtu_default)
					return 0;

				if (ethip->ip_header.packet_length > data_length)
					return 0;
				
				uint32_t max_packet_size = mtu_default + ethip->eth_header.length;
				uint8_t* frag_packet = static_cast<uint8_t*>(malloc(max_packet_size));
				const u_char* frag_packet_data = static_cast<u_char*>(frag_packet);

				if (frag_packet == 0)
					return 0;

				uint16_t ip_hd_length      = ethip->ip_header.get_length();
				uint16_t ip_payload_max    = mtu_default - ip_hd_length;
				uint32_t ip_payload_length = ethip->ip_header.packet_length - ip_hd_length;				
				
				uint16_t eth_ip_hd_length = ethip->header_length();
				uint8_t* current_data_offset = (uint8_t*)(data + eth_ip_hd_length);				
								
				//eth + ip headers
				if (memcpy_s(frag_packet, mtu_default, data, eth_ip_hd_length) != 0)
					return 0;
				
				ethernet_ipv4_header_t* fragment_ethip = ethernet_ipv4_header_t::get_header(frag_packet, 0);
				frag_packet += eth_ip_hd_length;
				uint16_t frag_packet_max_data = max_packet_size - eth_ip_hd_length;

				uint8_t ret = 0;
				uint32_t ip_fragment_offset = 0;

				while (ip_payload_length != 0)
				{
					if (ip_payload_length > ip_payload_max)
					{
						if (memcpy_s(frag_packet, frag_packet_max_data, current_data_offset, ip_payload_max) != 0)
						{
							ret = 0;
							break;
						}

						fragment_ethip->ip_header.packet_length = ip_hd_length + ip_payload_max;
						fragment_ethip->ip_header.set_fragment_offset(ip_fragment_offset);

						ip_payload_length -= ip_payload_max;
						current_data_offset += ip_payload_max;
						ip_fragment_offset += ip_payload_max;

						if (pcap_sendpacket(dst_if, frag_packet_data,
							fragment_ethip->ip_header.packet_length + ethip->eth_header.length) < 0)
						{
							ret = 0;
							break;
						}

						ret++;
					}
					else
					{
						if (memcpy_s(frag_packet, frag_packet_max_data, current_data_offset, ip_payload_length) != 0)
						{
							ret = 0;
							break;
						}

						fragment_ethip->ip_header.packet_length = ip_hd_length + ip_payload_length;
						fragment_ethip->ip_header.set_fragment_offset(ip_fragment_offset);
						fragment_ethip->ip_header.clear_more_fragments();
						ip_payload_length = 0;

						if (pcap_sendpacket(dst_if, frag_packet_data,
							fragment_ethip->ip_header.packet_length + ethip->eth_header.length) < 0)
						{
							ret = 0;
							break;
						}

						ret++;
					}
				}

				free((void*)frag_packet_data);
				return ret;
			}
		};
	}	
}
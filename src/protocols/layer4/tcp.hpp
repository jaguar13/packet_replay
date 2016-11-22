#pragma once
#include <stdint.h>
#include <endian/endianness.hpp>

namespace layer4 {
	namespace tcp {		

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
		}; 
		#pragma pack(pop)
	}	
}
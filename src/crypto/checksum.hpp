#pragma once 
#include "bittypes.h"

namespace crypto
{
	struct checksum_t
	{	
		 static uint16_t calc(uint16_t *data, int len)
		 {
           long sum = 0;  /* assume 32 bit long, 16 bit short */
           while(len > 1)
		   {
             sum += *data++;
             if(sum & 0x80000000)   /* if high order bit set, fold */
               sum = (sum & 0xFFFF) + (sum >> 16);
             
			 len -= 2;
           }

           if(len)       /* take care of left over byte */
             sum += *data;
          
           while(sum>>16)
             sum = (sum & 0xFFFF) + (sum >> 16);

           return static_cast<uint16_t>(~sum);
         }
	};
}


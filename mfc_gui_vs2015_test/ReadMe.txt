Done:
	- Bulk replaying on the interface. Replaying thread
	- CPU throlling.
	- Stop replaying
	- Disable Replay Bottom while replaying
	- Log File
	- Save settings.
	- Capture close event and stop the replaying thread.	
	- Fake replay for fragmented packets. Replay only the first packet.
	- TCP checksum on fragmented packet fixed.
	
Done (2017/02/14)
    - Fixed stats calculation at GUI.
	-  

Pending Features:
    - Sumit reported pcaps (3)
    - Command line recursive iteration.
	- Support Linux Cooked Capture.

    - Suport Mounted drive.	
	- IP editing (Avoid OS close packets)
	  - Require Interface selection.
	- File replay. Replay any file generating HTTP traffic
	
	- Ctrl+A on folder.
	- Realtime counters?	
	- TCP segmentation?

Pending BUGs:

***- Replay command line replaying on the same IP?
	- Refreshing network interfaces when an interface is installed after winpcap.
	- Server side rules are not detecting fragmented packets
	- Sumit reported PCAPS - HTTP.sys rule
	- Sumit reported problem
	- command line should say 0 pcap on empty folders, when the path has spaces.


	References:
	http://software-venupgopal.blogspot.ca/2016_12_01_archive.html
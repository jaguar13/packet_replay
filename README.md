Requirements:

Install wireshark ( https://www.wireshark.org/download.html ) or WinPCAP (https://www.winpcap.org/install/default.htm)


Version 1.2.6

    Fix:
        Some files are not replaying in share folders

    New Features:
        Delay configuration can added when replaying pcaps.

Version 1.2.4

    Fix:
        Some files are not replaying after compressed zip extraction
 

Version 1.2.3

    New Features:
        Bulk replaying on the interface. Replaying thread
        CPU throlling.
        Stop replaying
        Disable Replay Bottom while replaying
        Log File
        Save settings.
        Capture close event and stop the replaying thread.
        Fake replay for fragmented packets. Replay only the first packet.
        TCP checksum on fragmented packet fixed.

Version 1.2.2

Fixes:
    Checksum calculation on fragmented packets.

Version 1.2.1
    New Features:
        Recursive folder replay.
        Explorer Navigation: Back, Forward and Up
 

Version 1.2 
    New Features:

        Support Jumbo packet.
        Support certain types of packet corruption.
        Support folder replays.
        Dump statistics information about the replayed pcaps.
        Super easy configuration :-). Just IP selection from a list, the tool will automatically select the Interfaces with that IPs.
        Folder tree navigation Windows native.
        Replay Statistics:
            Total pcaps
            Corrupted pcaps
            Total packets
            Replayed packets
            Pcap with failed packets
            L2 non-supported packets
            Failed packet count       
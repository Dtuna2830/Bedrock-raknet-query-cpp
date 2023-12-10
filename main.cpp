#include <iostream>
#include <cstdint>
#include "raknet/RakPeerInterface.h"
#include "raknet/MessageIdentifiers.h"
#include "raknet/BitStream.h"
#include "raknet/GetTime.h"


int main() {
    std::locale turkish("turkish");
    std::locale::global(turkish);
    RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
    RakNet::SocketDescriptor sd;
    peer->Startup(1, &sd, 1);

    const char* serverIP = "play.redmc.me";
    const unsigned short serverPort = 19132;
    const int timeoutMilliseconds = 5000;

    RakNet::BitStream queryPacket;
    queryPacket.Write((RakNet::MessageID)ID_UNCONNECTED_PING);
    queryPacket.Write((uint64_t)RakNet::GetTime());

    peer->Ping(serverIP, serverPort, false);

    bool serverOnline = false;
    RakNet::Time startTime = RakNet::GetTime();

    // Wait for a response or timeout
    while (!serverOnline && RakNet::GetTime() - startTime < timeoutMilliseconds) {
        RakNet::Packet* packet;
        for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive()) {
            switch (packet->data[0]) {
            case ID_UNCONNECTED_PONG:
                if (packet->systemAddress == RakNet::SystemAddress(serverIP, serverPort)) {
                    unsigned long clientTimestamp;
                    unsigned long serverGUID;

                    memcpy(&clientTimestamp, packet->data + 1, sizeof(unsigned long));
                    memcpy(&serverGUID, packet->data + 5, sizeof(unsigned long));

                    std::string serverInfo(reinterpret_cast<char*>(packet->data + 9));

                    std::cout << "Client Timestamp: " << clientTimestamp << std::endl;
                    std::cout << "Server GUID: " << serverGUID << std::endl;
                    std::cout << "Server Information: " << serverInfo << std::endl;

                    serverOnline = true;
                }
                break;
            default:
                break;
            }
        }
    }

    if (!serverOnline) {
        std::cout << "Server is offline or unreachable." << std::endl;
    }

    RakNet::RakPeerInterface::DestroyInstance(peer);
    return 0;
}

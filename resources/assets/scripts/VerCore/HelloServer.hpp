#ifndef HELLOENGINE_HELLOSERVER_HPP
#define HELLOENGINE_HELLOSERVER_HPP

/*#include <iostream>
#include <thread>
#include <map>
#include <utility>
#include <vector>
#include <cstring>
#include <functional>
#include <unistd.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__) || defined(__unix__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

class C_HelloServer {
public:
    C_HelloServer(std::string  l_sIP, uint16_t l_iPORT, std::function<void(C_HelloServer*, std::string, sockaddr_in)> l_fRequestFunc, int l_iBufferSize = 1024)
            : l_sIP(std::move(l_sIP)), l_iPORT(l_iPORT), l_iBufferSize(l_iBufferSize), l_fRequestFunc(std::move(l_fRequestFunc))
    {

        if (this->l_iPORT <= 0 || this->l_iPORT > 65535)
        {
            exit(1);
        }

        this->l_iSockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (this->l_iSockfd == -1)
        {
            std::cerr << "Failed to create socket";
            exit(1);
        }

        this->l_sServerAddress.sin_family = AF_INET;
        inet_pton(AF_INET, this->l_sIP.c_str(), &(this->l_sServerAddress.sin_addr));
        this->l_sServerAddress.sin_port = htons(this->l_iPORT);
    }

    void F_vStart()
    {
        if (bind(this->l_iSockfd, (struct sockaddr*)&this->l_sServerAddress, sizeof(this->l_sServerAddress)) < 0)
        {
            std::cerr << "Bind failed";
            return;
        }

        this->l_tServerThread = std::thread(&C_HelloServer::f_vLoop, this);
        this->l_tServerThread.detach();
    }

    void F_vSend(const std::string& l_sMessage, const sockaddr_in& l_sAddress)
    {
        int l_iTotalSize = l_sMessage.length();
        std::vector<uint8_t> l_lMetaData(4);
        l_lMetaData[0] = (l_iTotalSize >> 24) & 0xFF;
        l_lMetaData[1] = (l_iTotalSize >> 16) & 0xFF;
        l_lMetaData[2] = (l_iTotalSize >> 8) & 0xFF;
        l_lMetaData[3] = l_iTotalSize & 0xFF;

        for (int l_iId = 0; l_iId < l_iTotalSize; l_iId += this->l_iBufferSize - 8)
        {
            std::string l_sChunk = l_sMessage.substr(l_iId, this->l_iBufferSize - 8);
            int l_iSequenceNumber = l_iId / (this->l_iBufferSize - 8);

            std::vector<uint8_t> l_lSequenceBytes(4);
            l_lSequenceBytes[0] = (l_iSequenceNumber >> 24) & 0xFF;
            l_lSequenceBytes[1] = (l_iSequenceNumber >> 16) & 0xFF;
            l_lSequenceBytes[2] = (l_iSequenceNumber >> 8) & 0xFF;
            l_lSequenceBytes[3] = l_iSequenceNumber & 0xFF;

            std::vector<uint8_t> l_lPacket;
            l_lPacket.insert(l_lPacket.end(), l_lMetaData.begin(), l_lMetaData.end());
            l_lPacket.insert(l_lPacket.end(), l_lSequenceBytes.begin(), l_lSequenceBytes.end());
            l_lPacket.insert(l_lPacket.end(), l_sChunk.begin(), l_sChunk.end());

            if (sendto(this->l_iSockfd, l_lPacket.data(), l_lPacket.size(), 0, (struct sockaddr*) &l_sAddress, sizeof(l_sAddress)) < 0)
            {
                std::cerr << "sendto() failed" << std::endl;
            }
        }
    }

    void F_vSendBroadCast(const std::string& l_sMessage)
    {
        for (sockaddr_in l_soClient : this->G_lClients)
        {
            this->F_vSend(l_sMessage, l_soClient);
        }
    }

    std::pair<std::string, sockaddr_in> F_pRecv()
    {
        std::string l_sMessage;
        sockaddr_in l_sAddress{};
        socklen_t addrlen = sizeof(l_sAddress);
        std::vector<std::vector<uint8_t>> l_lMessageParts;
        int l_iTotalSize = -1;

        while (true)
        {
            std::vector<uint8_t> l_lPacket(this->l_iBufferSize);

            int l_iBytesReceived = recvfrom(this->l_iSockfd, l_lPacket.data(), l_lPacket.size(), 0, (struct sockaddr*)&l_sAddress, &addrlen);

            if (l_iBytesReceived == -1)
            {
                std::cerr << "recvfrom() failed" << std::endl;
                return { nullptr, l_sAddress};
            }

            std::vector<uint8_t> l_lMetaData(l_lPacket.begin(), l_lPacket.begin() + 8);
            std::vector<uint8_t> l_lPayLoad(l_lPacket.begin() + 8, l_lPacket.begin() + l_iBytesReceived);

            if (l_iTotalSize == -1)
            {
                l_iTotalSize = 0;
                for (int l_iID = 0; l_iID < 4; ++l_iID)
                {
                    l_iTotalSize = (l_iTotalSize << 8) | l_lMetaData[l_iID];
                }
            }

            l_lMessageParts.push_back(l_lPayLoad);

            int l_iCurrentSize = 0;
            for (const std::vector<uint8_t>& l_lPart : l_lMessageParts)
            {
                l_iCurrentSize += l_lPart.size();
            }

            if (l_iCurrentSize >= l_iTotalSize)
            {
                break;
            }
        }

        for (const std::vector<uint8_t>& l_lPart : l_lMessageParts)
        {
            l_sMessage.append(l_lPart.begin(), l_lPart.end());
        }

        return { l_sMessage, l_sAddress};
    }

    void F_vAddClient(const sockaddr_in& address)
    {
        /*std::string ip = inet_ntoa(address.sin_addr);
        int port = ntohs(address.sin_port);
        this->l_mClients[ip].push_back(port);/
        this->G_lClients.push_back(address);
    }

    void F_vRemoveClient(const sockaddr_in& address)
    {
        std::string ip = inet_ntoa(address.sin_addr);
        int port = ntohs(address.sin_port);
        if (this->l_mClients.find(ip) != this->l_mClients.end()) {
            auto& ports = this->l_mClients[ip];
            ports.erase(std::remove(ports.begin(), ports.end(), port), ports.end());
            if (ports.empty()) {
                this->l_mClients.erase(ip);
            }
        }
    }

    void F_vStop()
    {
        close(this->l_iSockfd);
    }

private:
    void f_vLoop()
    {
        while (true) {
            try {
                auto received = this->F_pRecv();
                std::string message = received.first;
                sockaddr_in address = received.second;

                if (message == "CHECK") {
                    this->F_vSend("find", address);
                } else if (message == "CONNECT") {
                    this->F_vSend(std::to_string(l_iBufferSize), address);
                    this->F_vAddClient(address);
                } else {
                    std::thread requestThread(this->l_fRequestFunc, this, message, address);
                    requestThread.detach();
                }
            } catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }
        this->F_vStop();
    }

    std::string                                                     l_sIP;
    uint16_t                                                        l_iPORT;
    int                                                             l_iBufferSize = 1024;
    int                                                             l_iSockfd;
    struct sockaddr_in                                              l_sServerAddress;
    std::vector<sockaddr_in>                                        G_lClients;
    std::thread                                                     l_tServerThread;
    std::map<std::string, std::vector<int>>                         l_mClients;
    std::function<void(C_HelloServer*, std::string, sockaddr_in)>   l_fRequestFunc;
};*/

#include <iostream>
#include <thread>
#include <map>
#include <utility>
#include <vector>
#include <cstring>
#include <functional>
#include <fcntl.h>
#include <unistd.h>
#include "HelloWEBSOCKET.hpp"
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <thread>
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__) || defined(__unix__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#endif


#if defined(PLATFORM_DESKTOP)
#include <condition_variable>
#include <sstream>
#include <thread>
#endif

namespace HelloServer
{
    struct S_Client
    {
        int         G_iTCP  = -1;
        sockaddr_in G_soUDP{};
    };
}

extern "C" class C_HelloServer {
private:
    std::string                                                     G_sIP;
    uint16_t                                                        G_iPORT;
    int                                                             G_iBufferSize = 1024;
    int                                                             G_iSockfdUDP;
    HelloWS*                                                        G_cWebSocket;
    struct sockaddr_in                                              G_sServerAddressUDP;
    std::function<void(C_HelloServer*, std::string)>                G_fRequestFunc;
    bool                                                            G_bRunning = false;
    std::vector<std::unique_ptr<std::thread>>                       G_vThreads;


    void F_vStartUDP()
    {
        this->G_iSockfdUDP = socket(AF_INET, SOCK_DGRAM, 0);
        if (this->G_iSockfdUDP == -1)
        {
            std::cerr << "Failed to create socket";
            exit(1);
        }

        this->G_sServerAddressUDP.sin_family = AF_INET;
        this->G_sServerAddressUDP.sin_addr.s_addr = inet_addr(this->G_sIP.c_str());
        this->G_sServerAddressUDP.sin_port = htons(this->G_iPORT);

        int yes = 1;
        if (setsockopt(this->G_iSockfdUDP, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
            std::cerr << "setsockopt SO_REUSEADDR error";
            exit(EXIT_FAILURE);
        }

        if (bind(this->G_iSockfdUDP, (struct sockaddr*)&this->G_sServerAddressUDP, sizeof(this->G_sServerAddressUDP)) < 0)
        {
            TraceLog(LOG_ERROR, "ServerSocket: Failed to connect");
            exit(EXIT_FAILURE);
        }
    }

    void F_vHandleUDP()
    {
        while (this->G_bRunning) {
            try {
                auto received = this->F_pRecv();
                std::string message = received.first;
                sockaddr_in address = received.second;

                if (message.substr(0, 5) == "CHECK") {
                    this->G_mClients[0] = {-1, address};
                    this->F_vSend("find", 0);
                } else if (message.substr(0, 10) == "DISCONNECT") {
                    std::vector<std::string> l_lSplitMSG = C_Utils::F_lSplit(message, ":");
                    std::cout << l_lSplitMSG[1] << std::endl;
                    this->F_vRemoveClient((int) atoi(l_lSplitMSG[1].c_str()));
                } else if (message.substr(0, 7) == "CONNECT") {
                    int l_iIdClient = this->G_mClients.size();
                    this->F_vAddClient((HelloServer::S_Client) {-1, address});
                    this->F_vSend(std::to_string(this->G_iBufferSize), l_iIdClient);
                    this->F_vSend(std::to_string(l_iIdClient), l_iIdClient);
                } else {
                    std::thread requestThread(this->G_fRequestFunc, this, message);
                    requestThread.detach();
                }
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
        }
    }

public:
    std::vector<HelloServer::S_Client>                              G_mClients = { HelloServer::S_Client() };

    C_HelloServer(std::string  l_sIP, uint16_t l_iPORT, std::function<void(C_HelloServer*, std::string)> l_fRequestFunc, int l_iBufferSize = 1024)
            : G_sIP(std::move(l_sIP)), G_iPORT(l_iPORT), G_iBufferSize(l_iBufferSize), G_fRequestFunc(std::move(l_fRequestFunc))
    {

        if (this->G_iPORT <= 0 || this->G_iPORT > 65535)
        {
            exit(1);
        }

        this->G_cWebSocket = new HelloWS(this->G_sIP, this->G_iPORT+1);
        this->F_vStartUDP();
    }

    ~C_HelloServer() {
        std::cout << "closed" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void F_vStart()
    {
        this->G_bRunning = true;

        this->G_vThreads.push_back(std::make_unique<std::thread>([&]()
        {

            this->F_vHandleUDP();

        }));

        this->G_vThreads.push_back(std::make_unique<std::thread>([&](){
            this->G_cWebSocket->run([&](HelloWS *self, int clientSocket, const std::string& client_address, int client_port)
            {
                while (this->G_bRunning)
                {
                    try
                    {
                        std::string message = self->recv(clientSocket);
                        if(message.empty())
                            continue;

                        if (message.substr(0, 5) == "CHECK")
                        {
                            this->G_cWebSocket->send(clientSocket, "find");
                        } else if (message.substr(0, 10) == "DISCONNECT")
                        {
                            break;
                        } else if (message.substr(0, 7) == "CONNECT")
                        {
                            int l_iIdClient = this->G_mClients.size();
                            this->F_vAddClient({clientSocket});
                            this->F_vSend("CONNECT1:" + std::to_string(this->G_iBufferSize), l_iIdClient);
                            this->F_vSend("CONNECT2:" + std::to_string(l_iIdClient), l_iIdClient);
                        } else
                        {
                            std::thread requestThread(this->G_fRequestFunc, this, message);
                            requestThread.detach();
                        }
                    } catch (std::exception &e)
                    {
                        std::cerr << e.what() << std::endl;
                    }
                }

#if defined(_WIN32) || defined(_WIN64)
                closesocket(clientSocket);
#else
                close(clientSocket);
#endif
            });
        }));

        for (auto& l_thThread : this->G_vThreads)
        {
            l_thThread->detach();
        }
    }

    void F_vSend(const std::string& l_sMessage, int l_iIdClient)
    {
        HelloServer::S_Client l_sAddress = this->G_mClients[l_iIdClient];

        if(l_sAddress.G_iTCP != -1)
        {
            this->G_cWebSocket->send(l_sAddress.G_iTCP, l_sMessage);
        } else
        {
            int l_iTotalSize = l_sMessage.length();
            std::vector<uint8_t> l_lMetaData(4);
            l_lMetaData[0] = (l_iTotalSize >> 24) & 0xFF;
            l_lMetaData[1] = (l_iTotalSize >> 16) & 0xFF;
            l_lMetaData[2] = (l_iTotalSize >> 8) & 0xFF;
            l_lMetaData[3] = l_iTotalSize & 0xFF;

            for (int l_iId = 0; l_iId < l_iTotalSize; l_iId += this->G_iBufferSize - 8) {
                std::string l_sChunk = l_sMessage.substr(l_iId, this->G_iBufferSize - 8);
                int l_iSequenceNumber = l_iId / (this->G_iBufferSize - 8);

                std::vector<uint8_t> l_lSequenceBytes(4);
                l_lSequenceBytes[0] = (l_iSequenceNumber >> 24) & 0xFF;
                l_lSequenceBytes[1] = (l_iSequenceNumber >> 16) & 0xFF;
                l_lSequenceBytes[2] = (l_iSequenceNumber >> 8) & 0xFF;
                l_lSequenceBytes[3] = l_iSequenceNumber & 0xFF;

                std::vector<uint8_t> l_lPacket;
                l_lPacket.insert(l_lPacket.end(), l_lMetaData.begin(), l_lMetaData.end());
                l_lPacket.insert(l_lPacket.end(), l_lSequenceBytes.begin(), l_lSequenceBytes.end());
                l_lPacket.insert(l_lPacket.end(), l_sChunk.begin(), l_sChunk.end());

                if (sendto(this->G_iSockfdUDP, (const char*) l_lPacket.data(), l_lPacket.size(), 0,
                           (struct sockaddr *) &l_sAddress.G_soUDP, sizeof(l_sAddress.G_soUDP)) < 0) {
                    //std::cerr << "sendto() failed" << std::endl;
                }
            }
        }
    }

    void F_vSendBroadCast(const std::string& l_sMessage)
    {
        for (int l_iId = 0; l_iId < this->G_mClients.size(); ++l_iId)
        {
            this->F_vSend(l_sMessage, l_iId);
        }
    }

    std::pair<std::string, sockaddr_in> F_pRecv(int l_iClientSocket=-1)
    {
        if(l_iClientSocket != -1)
        {
            sockaddr_in l_sAddress{};
            socklen_t addrlen = sizeof(l_sAddress);

            return { this->G_cWebSocket->recv(l_iClientSocket), l_sAddress };
        } else {
            std::string l_sMessage;
            sockaddr_in l_sAddress{};
            socklen_t addrlen = sizeof(l_sAddress);
            std::vector<std::vector<uint8_t>> l_lMessageParts;
            int l_iTotalSize = -1;

            while (this->G_bRunning) {
                std::vector<uint8_t> l_lPacket(this->G_iBufferSize);

                int l_iBytesReceived = -1;
                l_iBytesReceived = recvfrom(this->G_iSockfdUDP, (char*) l_lPacket.data(), l_lPacket.size(), 0,
                                            (struct sockaddr *) &l_sAddress, &addrlen);

                if (l_iBytesReceived == -1) {
                    //std::cerr << "recvfrom() failed" << std::endl;
                    return {nullptr, l_sAddress};
                }

                std::vector<uint8_t> l_lMetaData(l_lPacket.begin(), l_lPacket.begin() + 8);
                std::vector<uint8_t> l_lPayLoad(l_lPacket.begin() + 8, l_lPacket.begin() + l_iBytesReceived);

                if (l_iTotalSize == -1) {
                    l_iTotalSize = 0;
                    for (int l_iID = 0; l_iID < 4; ++l_iID) {
                        l_iTotalSize = (l_iTotalSize << 8) | l_lMetaData[l_iID];
                    }
                }

                l_lMessageParts.push_back(l_lPayLoad);

                int l_iCurrentSize = 0;
                for (const std::vector<uint8_t> &l_lPart: l_lMessageParts) {
                    l_iCurrentSize += l_lPart.size();
                }

                if (l_iCurrentSize >= l_iTotalSize) {
                    break;
                }
            }

            for (const std::vector<uint8_t> &l_lPart: l_lMessageParts) {
                l_sMessage.append(l_lPart.begin(), l_lPart.end());
            }

            return {l_sMessage, l_sAddress};
        }
    }

    void F_vAddClient(HelloServer::S_Client l_sClient)
    {
        this->G_mClients.push_back(l_sClient);
    }
    void F_vRemoveClient(int l_iId)
    {
        if (l_iId < this->G_mClients.size()) {
            this->G_mClients.erase(this->G_mClients.begin() + l_iId);
        }
    }

    void F_vStop()
    {
        this->G_bRunning = false;
        close(this->G_iSockfdUDP);
        this->G_cWebSocket->close_server();
    }
};

#endif

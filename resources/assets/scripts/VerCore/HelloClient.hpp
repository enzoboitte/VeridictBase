#ifndef HELLOENGINE_HELLOCLIENT_HPP
#define HELLOENGINE_HELLOCLIENT_HPP

/*#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <sys/types.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__) || defined(__unix__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <unistd.h>
#include <utility>
#include <vector>

class C_HelloClient
{
public:

    C_HelloClient(std::string  l_sIP, int l_iPORT, std::function<void(C_HelloClient*, std::string)> l_fRequestFunc)
            : l_sIP(std::move(l_sIP)), l_iPORT(l_iPORT), l_fRequestFunc(std::move(l_fRequestFunc))
    {
        if (this->l_iPORT < 0 || this->l_iPORT > 65535)
        {
            std::exit(1);
        }

        this->l_iSockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (this->l_iSockfd == -1)
        {
            std::cerr << "Failed to create socket";
            std::exit(1);
        }

        this->l_sServerAddress.sin_family = AF_INET;
        this->l_sServerAddress.sin_port = htons(this->l_iPORT);
#if defined(_WIN32) || defined(_WIN64)
        inet_ntoa(AF_INET, this->l_sIP.c_str(), &this->l_sServerAddress.sin_addr);
#else
        inet_pton(AF_INET, this->l_sIP.c_str(), &this->l_sServerAddress.sin_addr);
#endif

        this->l_bStopFlag = false;
    }

    void F_vStart() {
        try
        {
            this->F_vSend("CONNECT");
            std::string buff = this->F_sRecv();
            this->l_iBufferSize = std::stoi(buff);
            std::thread receiveThread(&C_HelloClient::f_vLoop, this);
            receiveThread.detach();
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what();
        }
    }

    void F_vSend(const std::string& l_sMessage)
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

            if (sendto(this->l_iSockfd, l_lPacket.data(), l_lPacket.size(), 0, (struct sockaddr*) &this->l_sServerAddress, sizeof(this->l_sServerAddress)) < 0)
            {
                std::cerr << "sendto() failed" << std::endl;
            }
        }
    }

    std::string F_sRecv()
    {
        std::string l_sMessage;
        std::vector<std::vector<uint8_t>> l_lMessageParts;
        int l_iTotalSize = -1;

        while (true)
        {
            std::vector<uint8_t> l_lPacket(this->l_iBufferSize);
            socklen_t l_sAddrLen = sizeof(this->l_sServerAddress);

            int l_iBytesReceived = recvfrom(this->l_iSockfd, l_lPacket.data(), l_lPacket.size(), 0, (struct sockaddr*)&this->l_sServerAddress, &l_sAddrLen);

            if (l_iBytesReceived == -1)
            {
                std::cerr << "recvfrom() failed" << std::endl;
                return nullptr;
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

        return l_sMessage;
    }

    void F_vStop()
    {
        this->l_bStopFlag = true;
        close(this->l_iSockfd);
    }



    std::string                                         l_sIP;

    uint16_t                                            l_iPORT;
    int                                                 l_iSockfd;
    int                                                 l_iBufferSize       = 1024;
    struct sockaddr_in                                  l_sServerAddress{};
    bool                                                l_bStopFlag;
    std::function<void(C_HelloClient*, std::string)>    l_fRequestFunc;

private:
    void f_vLoop()
    {
        while (!this->l_bStopFlag)
        {
            try
            {
                std::string message = this->F_sRecv();
                std::thread requestThread(this->l_fRequestFunc, this, message);
                requestThread.detach();
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what();
            }
        }
    }
};*/

#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include <chrono>

#if defined(_WIN32) || defined(_WIN64)
#include "winsock2.h"
#include "ws2tcpip.h"

#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__) || defined(__unix__) || defined(PLATFORM_WEB)
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>
#include <emscripten/threading.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

extern "C" class C_HelloClient
{
public:
    int G_iId = -1;

    C_HelloClient(std::string  l_sIP, int l_iPORT, std::function<void(C_HelloClient*, std::string)> l_fRequestFunc)
            : l_sIP(std::move(l_sIP)), l_iPORT(l_iPORT), l_fRequestFunc(std::move(l_fRequestFunc))
    {
    #if defined(PLATFORM_WEB)
        G_bIsTCP = true;

        EmscriptenWebSocketCreateAttributes attr;
        emscripten_websocket_init_create_attributes(&attr);

        attr.url = ("ws://" + this->l_sIP + ":" + std::to_string(this->l_iPORT+1)).c_str();

        ws = emscripten_websocket_new(&attr);
        if (ws <= 0)
        {
            std::cerr << "Failed to create WebSocket.\n";
            std::exit(1);
        }

        emscripten_websocket_set_onopen_callback(ws, this, onopen);
        emscripten_websocket_set_onerror_callback(ws, this, onerror);
        emscripten_websocket_set_onclose_callback(ws, this, onclose);
        emscripten_websocket_set_onmessage_callback(ws, this, onmessage);

        emscripten_sleep(100); // Cède le contrôle au navigateur pour éviter de le bloquer complètement
        while (!this->G_bIsOpened) {
            emscripten_sleep(100); // Attend activement en cédant le contrôle au navigateur
        }
    #else

        if (this->l_iPORT < 0 || this->l_iPORT > 65535)
        {
            std::exit(1);
        }

        this->l_iSockfd = socket(AF_INET, SOCK_DGRAM , 0);
        if (this->l_iSockfd == -1)
        {
            std::cerr << "Failed to create socket";
            std::exit(1);
        }

        this->l_sServerAddress.sin_family = AF_INET;
        this->l_sServerAddress.sin_port = htons(this->l_iPORT + (this->G_bIsTCP ? 1 : 0));
        this->l_sServerAddress.sin_addr.s_addr = inet_addr(this->l_sIP.c_str());
#endif
        this->l_bStopFlag = false;

    }

    ~C_HelloClient() {
#if defined(PLATFORM_WEB)
        emscripten_websocket_close(ws, 1000, "Closing connection");
        emscripten_websocket_delete(ws);
#endif
    }

    void F_vStart() {
        if(!this->G_bIsTCP)
        {
            try {
                this->F_vSend("CONNECT");
                std::string buff = this->F_sRecv();
                this->l_iBufferSize = std::stoi(buff);

                buff = this->F_sRecv();
                this->G_iId = atoi(buff.c_str());

                std::thread receiveThread(&C_HelloClient::f_vLoop, this);
                receiveThread.detach();
            }
            catch (const std::exception &e) {
                std::cerr << e.what();
            }
        } else
        {
            /*auto l_aStart = std::chrono::steady_clock::now();
            while (!this->G_bIsOpened)
            {
                auto l_aEnd = std::chrono::steady_clock::now();
                auto l_aElapsed = std::chrono::duration_cast<std::chrono::seconds>(l_aEnd - l_aStart);

                if(l_aElapsed.count() >= 10)
                {
                    TraceLog(LOG_ERROR, "Time out to connect server");
                    exit(1);
                }
            }*/
        }
    }

    void F_vSend(std::string l_sMessage, bool l_bWithId = true)
    {
        if(l_bWithId)
            l_sMessage += "\nid_client=" + std::to_string(G_iId);

#if defined(PLATFORM_WEB)
        if(this->G_bIsOpened)
        {
            EMSCRIPTEN_RESULT result = emscripten_websocket_send_utf8_text(ws, l_sMessage.c_str());
            if (result) {
                printf("Failed to emscripten_websocket_send_utf8_text(): %d\n", result);
            }
        } else
        {
            printf("Failed to send because client connection not opened\n");
        }
#else
        int l_iTotalSize = l_sMessage.length();
        std::vector<uint8_t> l_lMetaData(4);
        l_lMetaData[0] = (l_iTotalSize >> 24) & 0xFF;
        l_lMetaData[1] = (l_iTotalSize >> 16) & 0xFF;
        l_lMetaData[2] = (l_iTotalSize >> 8) & 0xFF;
        l_lMetaData[3] = l_iTotalSize & 0xFF;

        for (int l_iId = 0; l_iId < l_iTotalSize; l_iId += this->l_iBufferSize - 8) {
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

            if (sendto(this->l_iSockfd, (const char*) l_lPacket.data(), l_lPacket.size(), 0,
                       (struct sockaddr *) &this->l_sServerAddress, sizeof(this->l_sServerAddress)) < 0) {
                //std::cerr << "sendto() failed" << std::endl;
            }
        }
#endif
    }

    std::string F_sRecv()
    {
        std::string l_sMessage;

        if(!this->G_bIsTCP)
        {
            std::vector<std::vector<uint8_t>> l_lMessageParts;
            int l_iTotalSize = -1;

            while (true) {
                std::vector<uint8_t> l_lPacket(this->l_iBufferSize);
                socklen_t l_sAddrLen = sizeof(this->l_sServerAddress);
                int l_iBytesReceived = recvfrom(this->l_iSockfd, (char *) l_lPacket.data(), l_lPacket.size(), 0,
                                            (struct sockaddr *) &this->l_sServerAddress, &l_sAddrLen);

                if (l_iBytesReceived == -1) {
                    //std::cerr << "recvfrom() failed" << std::endl;
                    return nullptr;
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
        }

        return l_sMessage;
    }

    void F_vStop()
    {
        this->F_vSend("DISCONNECT:" + std::to_string(G_iId), false);
        this->l_bStopFlag = true;
        close(this->l_iSockfd);
    }



    std::string                                         l_sIP;

    uint16_t                                            l_iPORT;
    int                                                 l_iSockfd;
    int                                                 l_iBufferSize       = 1024;
    struct sockaddr_in                                  l_sServerAddress{};
    bool                                                l_bStopFlag;
    bool                                                G_bIsTCP = false;
    bool                                                G_bIsOpened = false;

    std::function<void(C_HelloClient*, std::string)>    l_fRequestFunc;

#if defined(PLATFORM_WEB)
    EMSCRIPTEN_WEBSOCKET_T ws;

    static EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData) {
        std::cout << "Connection opened\n";
        auto* client = static_cast<C_HelloClient*>(userData);

        client->ws = e->socket;
        client->G_bIsOpened = true;

        client->F_vSend("CONNECT");
        return EM_TRUE;
    }

    static EM_BOOL onerror(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData) {
        std::cerr << "Error occurred\n";
        return EM_FALSE;
    }

    static EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData) {
        std::cout << "Connection closed\n";
        auto* client = static_cast<C_HelloClient*>(userData);

        client->G_bIsOpened = false;
        return EM_TRUE;
    }

    static EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData) {
        auto* client = static_cast<C_HelloClient*>(userData);

        client->ws = e->socket;
        if (e->isText) {
            std::string message((const char*) e->data, e->numBytes);

            size_t pos;
            if ((pos = message.find("CONNECT1:", 0)) != std::string::npos) {
                message.erase(pos, 9);
                client->l_iBufferSize = std::stoi(message);
            } else if ((pos = message.find("CONNECT2:", 0)) != std::string::npos) {
                message.erase(pos, 9);
                client->G_iId = atoi(message.c_str());
            } else
            {
                try
                {
                    client->l_fRequestFunc(client, message);
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                }
            }
        }
        return EM_TRUE;
    }
#endif
private:
    void f_vLoop()
    {
        while (!this->l_bStopFlag)
        {
            try
            {
                std::string message = this->F_sRecv();
                std::thread requestThread(this->l_fRequestFunc, this, message);
                requestThread.detach();
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what();
            }
        }
    }
};

#endif

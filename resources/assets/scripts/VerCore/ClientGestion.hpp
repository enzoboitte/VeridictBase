#ifndef HELLOENGINE_SERVERSTATE_H
#define HELLOENGINE_SERVERSTATE_H

#include <cstdint>
#include <utility>
#include "./HelloClient.hpp"
#include "./HelloServer.hpp"
#include "./Tools.hpp"

inline void F_vFunctionAction(C_HelloClient* l_cClient, const std::string& l_sMSG);

class C_ClientGestion {
private:
    bool        G_bIsServerCreated = false;
    std::string G_sIP;
    uint16_t    G_uiPort{};

public:
    std::map<std::string, std::function<void(C_HelloClient*, std::string)>> G_lFunctionActions;
    C_HelloClient* G_cHelloClient = nullptr;

    C_ClientGestion() = default;
    C_ClientGestion(const C_ClientGestion&) = delete;
    virtual ~C_ClientGestion() = default;

    void F_vSetState(bool l_bState)
    {
        G_bIsServerCreated = l_bState;
    }

    [[nodiscard]] bool F_bGetState() const
    {
        return G_bIsServerCreated;
    }

    void F_vSetPort(uint16_t l_uiPort)
    {
        this->G_uiPort = l_uiPort;
    }
    [[nodiscard]] uint16_t L_fGetPort() const
    {
        return G_uiPort;
    }

    void F_vSetIP(std::string l_sIP)
    {
        this->G_sIP = std::move(l_sIP);
    }
    std::string F_sGetPort()
    {
        return G_sIP;
    }

    void F_vConnect()
    {
        if (this->G_sIP.empty() || (this->G_uiPort < 0 || this->G_uiPort > 65535)) return;
        this->G_cHelloClient = new C_HelloClient(this->G_sIP, this->G_uiPort, F_vFunctionAction);
        this->G_cHelloClient->F_vStart();
    }

    void F_vDisconnect()
    {
        if(this->G_cHelloClient == nullptr) return;
        this->G_cHelloClient->F_vStop();
    }

    const std::function<void(C_HelloClient *, std::string)>& F_lGetFunctionByActions(const std::string& l_sAction)
    {
        if(this->G_cHelloClient == nullptr) return nullptr;
        return this->G_lFunctionActions[l_sAction];
    }

    void F_vSetFunctionByActions(const std::string& l_sAction, const std::function<void(C_HelloClient *, std::string)>& gLFunctionActions)
    {
        if(this->G_cHelloClient == nullptr) return;
        this->G_lFunctionActions[l_sAction] = gLFunctionActions;
    }

    void F_vRemoveFunctionByActions(const std::string& l_sAction)
    {
        if(this->G_cHelloClient == nullptr) return;
        this->G_lFunctionActions.erase(l_sAction);
    }

    void F_vRemoveAllFunctions()
    {
        if(this->G_cHelloClient == nullptr) return;
        this->G_lFunctionActions.clear();
    }
};

inline auto G_cClientGestion = std::make_unique<C_ClientGestion>();
inline void F_vFunctionAction(C_HelloClient* l_cClient, const std::string& l_sMSG)
{
    std::string l_sAction = VerTools::F_sParseNetworkMSG(l_sMSG)["action"];

    if(G_cClientGestion->G_lFunctionActions.count(l_sAction) == 0) {
        return;
    }

    G_cClientGestion->F_lGetFunctionByActions(l_sAction)(l_cClient, l_sMSG);
}

#endif

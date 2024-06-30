#ifndef HELLOENGINE_SPRITEGROUP_HPP
#define HELLOENGINE_SPRITEGROUP_HPP

#include <vector>
#include <algorithm>
#include <memory>
#include <map>

#include "raylib.h"
#include "./IReflectable.hpp"
#include "./SceneManager.hpp"

/*#ifdef UBODY
    #undef UBODY
    #define UBODY() \
    public: \
        Reflectable reflectable; \
        C_GameObject* GameObject;

#endif*/

class C_GameObject;

class C_SpriteGroup
{
private:
    std::map<long long int, std::shared_ptr<C_GameObject>> G_lSprites;

public:
    void F_vDestroyAll();
    void F_vAddSprite(std::shared_ptr<C_GameObject>&& l_cSprite);

    void F_vUpdate();
    void F_vDraw();
    void F_vDrawScreen();

    const std::map<long long int, std::shared_ptr<C_GameObject>>& F_vGetSprites();
};

class ObjectBehaviour : public Reflectable
{
    UBODY()
    UPROPERTY()

public:
    explicit ObjectBehaviour()
    {
        //RegisterProperty("GameObject", &this->GameObject);
    }

    virtual void F_vUpdate(C_GameObject* l_cGameObjet);
    virtual void F_vDraw(...);
    virtual void F_vDrawScreen(...);
    virtual void F_vUnInit(...);
};

class C_Transform : public ObjectBehaviour
{
UBODY()
UPROPERTY()
    //Vector2         G_v2Velocity{};

    Vector3         G_v3Position{};
    Vector3         G_v3Scale   {};
    Vector3         G_v3Rotation{};
public:
    explicit C_Transform()
    {
        this->RegisterProperty<Vector3>("position", &this->G_v3Position);
        this->RegisterProperty<Vector3>("rotation", &this->G_v3Rotation);
        this->RegisterProperty<Vector3>("scale", &this->G_v3Scale);
    };

    //const Vector2&  F_v2GetVelocity() const;
    //void            F_vSetVelocity(const Vector2 &l_v2Velocity);

    const Vector3&  F_v2GetPosition() const;
    void            F_vSetPosition(const Vector3 &l_v2Position);
    const Vector3&  F_v2GetScale() const;
    void            F_vSetScale(const Vector3 &l_v2Scale);
    const Vector3&  F_v2GetRotation() const;
    void            F_vSetRotation(const Vector3 &l_v2Rotation);
    Vector3         F_v2GetPositionCenter();
};

class C_GameObject : public Reflectable
{
UBODY()
UPROPERTY()

    std::string G_sName     = "";
    int         G_iTag      = 0;
    std::string G_sType     = "";
    bool        G_bDestroy  = false;

public:
    C_SceneManager*   G_cSceneManeger{};
    C_SpriteGroup*    G_cSpriteGroup{};

    std::map<std::string, std::shared_ptr<Reflectable>> G_mComponent{};
    std::vector<std::shared_ptr<Reflectable>> G_lChild{};

    C_GameObject()
    {
        //this->G_mComponent.emplace("C_Transform", std::make_shared<C_Transform>());

        this->RegisterProperty<std::string>("name", &this->G_sName);
        this->RegisterProperty<int>("tag", &this->G_iTag);
        this->RegisterProperty<std::string>("type", &this->G_sType);
        this->RegisterProperty<bool>("destroy", &this->G_bDestroy);
        this->RegisterProperty<std::map<std::string, std::shared_ptr<Reflectable>>>("components", &this->G_mComponent);
        this->RegisterProperty<std::vector<std::shared_ptr<Reflectable>>>("childs", &this->G_lChild);
    };

    void F_vSetSceneManeger(C_SceneManager *gCSceneManeger);
    void F_vSetSpriteGroup(C_SpriteGroup *gCSpriteGroup);

    virtual void F_vUpdate(...);
    virtual void F_vDraw(...);
    virtual void F_vDrawScreen(...);
    virtual void F_vUnInit(...);

    virtual void F_vOnCollide(std::shared_ptr<C_GameObject>& l_cOther);

    void            F_vSetTag(long long int l_iTag);
    long long int   F_iGetTag();
    void            F_vDestroy();
    bool            F_bIsDestory() const;
};

#include "Camera.hpp"
#endif

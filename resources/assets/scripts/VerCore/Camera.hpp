#ifndef HELLOENGINE_CAMERA_HPP
#define HELLOENGINE_CAMERA_HPP

#include "./SpriteGroup.hpp"
#include "raylib.h"

class C_Camera : public ObjectBehaviour {
public:
    Camera3D G_c3Camera{};

    explicit C_Camera() {
        this->G_c3Camera.position = (Vector3) { 0, 0, 0 };
        this->G_c3Camera.target = (Vector3) { 0, 0, 0 };
        this->G_c3Camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
        this->G_c3Camera.fovy = 45.0f;
        this->G_c3Camera.projection = CAMERA_PERSPECTIVE;

        RegisterProperty<Camera3D>("camera", &this->G_c3Camera);
    };

    void F_vUpdate(C_GameObject* l_cGameObjet) override;

    void F_vShake(int l_iDuration, int l_iSpeed);
};


#endif

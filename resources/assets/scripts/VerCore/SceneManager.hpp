#ifndef HELLOENGINE_SCENEMANAGER_HPP
#define HELLOENGINE_SCENEMANAGER_HPP

#include <utility>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>
#include <raylib.h>
#include <functional>
#include <raymath.h>
#include <iostream>
#include <cstring>
#include <memory>

#define USCENEDECL() \
inline int G_iNScene   = 0; \
inline StringList G_lScene; \
                \
__attribute__((constructor)) \
void Init_Lib_Scene() \
{                 \
    initStringList(&G_lScene); \
}               \
\
extern "C" int GetNbScene() \
{ return G_lScene.size; } \
                \
extern "C" StringList GetListScene() \
{ return G_lScene; }

#define USCENE(C, I) \
__attribute__((constructor)) \
void MAKE_UNIQUE(C)() \
{                 \
    push_back(&G_lScene, quote(C)); \
    G_iNScene++; \
} \
extern "C" const unsigned char* CONCATENATE(F_scn_, C)() \
{ return C; } \
extern "C" const int CONCATENATE(F_len_, C)() \
{ return I; }


class C_SceneManager;

class C_Scene
{
public:
    class C_SceneManager*   G_cSceneManeger;

    explicit C_Scene(C_SceneManager* l_cSceneManeger) : G_cSceneManeger(l_cSceneManeger) {}

    virtual void F_vInit();
    virtual void F_vUpdate();
    virtual void F_vDraw();
    virtual void F_vUnInit();
};

class C_SceneManager
{
public:
    std::unique_ptr<C_Scene>    G_cCurrentScene;

    C_SceneManager() : G_cCurrentScene(nullptr) {}

    void F_vSetScene(std::unique_ptr<C_Scene> l_cScene);

    void F_vUpdate();
    void F_vDraw();
    void F_vUnInit();
};

class C_Component
{
public:
    explicit C_Component() = default;

    virtual void F_vUpdate();
    virtual void F_vDraw();
};

class C_Display
{
public:
    C_Display(std::vector<C_Component *> l_vListComponents) : l_vListComponents(std::move(l_vListComponents)) {};

    void F_vUpdate();
    void F_vDraw();
private:
    std::vector<C_Component *> l_vListComponents;
};

class C_GUI
{
public:

    C_GUI() = default;

    void F_vInit(std::map<std::string, C_Display *> l_mListDisplay, std::string l_sDefaultDisplay);
    void F_vShowLastDisplay();
    void F_vShowDisplayByName(const std::string& l_sName);

    void F_vUpdate();
    void F_vDraw();

private:
    std::map<std::string, C_Display *>  l_mListDisplay;
    std::string                         l_sCurrentDisplay;
    std::vector<std::string>            l_vLastDisplay;
};

class C_Button : public C_Component
{
private:
    Color l_cColorDefault;
    Color l_cColorHover;
    Color l_cColorPressed;

    std::string                     l_sText;
    float                           l_fSize;
    std::function<Vector2(void)>    l_fPosition;
    Vector2 l_v2LastPosition;

    Vector2 l_v2Position;
    int l_iScreenWidth;

    int l_iScreenHeight;
    bool l_bHover       = false;
    bool l_bFullScreen  = false;

    bool l_bPressed = false;
    Sound l_sSoundClick;

    Font  l_fFont;

    std::function<void(void)>    G_fAction;
public:

    explicit C_Button(std::string l_sText, float l_fSize, std::function<Vector2(void)> l_fPosition, std::function<void(void)> l_fAction= nullptr, Color l_cColor=RAYWHITE)
        : l_sText(std::move(l_sText)), l_fSize(l_fSize),
          l_fPosition(std::move(l_fPosition)),

          G_fAction(std::move(l_fAction)),

          l_cColorDefault(l_cColor),
          l_cColorHover((Color) {static_cast<unsigned char>(l_cColor.r >> 1), static_cast<unsigned char>(l_cColor.g >> 1), static_cast<unsigned char>(l_cColor.b >> 1), 255}),
          l_cColorPressed((Color) {static_cast<unsigned char>(l_cColor.r >> 2), static_cast<unsigned char>(l_cColor.g >> 2), static_cast<unsigned char>(l_cColor.b >> 2), 255})
    {
        this->l_v2LastPosition  = this->l_fPosition();
        this->l_v2Position      = { this->l_v2LastPosition.x, this->l_v2LastPosition.y };

        this->l_iScreenWidth    = GetScreenWidth();
        this->l_iScreenHeight   = GetScreenHeight();

        this->l_sSoundClick     = LoadSound(("./resources/assets/music/ui/Abstract1.wav"));
        this->l_fFont           = GetFontDefault();//LoadFont("./resources/assets/font/m3x6.ttf");
        this->l_bFullScreen     = IsWindowFullscreen();
    };

    bool F_bIsCollide(Vector2 l_v2Point)
    {
        Vector2 l_v2SizeText = MeasureTextEx(GetFontDefault(), this->l_sText.c_str(), this->l_fSize, 1);
        return CheckCollisionPointRec((Vector2) l_v2Point, (Rectangle){ this->l_v2LastPosition.x - 10, this->l_v2LastPosition.y - 10, (l_v2SizeText.x) + 20, (l_v2SizeText.y) + 20 });
    }

    bool F_bIsPressed(Vector2 l_v2Point)
    {
        return this->F_bIsCollide(l_v2Point) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    void F_vUpdate() override
    {
        int l_iTempScreenWidth    = GetScreenWidth();
        int l_iTempScreenHeight   = GetScreenHeight();

        if (this->l_iScreenWidth != l_iTempScreenWidth || this->l_iScreenHeight != l_iTempScreenHeight || this->l_bFullScreen != IsWindowFullscreen())
        {
            this->l_iScreenWidth    = l_iTempScreenWidth;
            this->l_iScreenHeight   = l_iTempScreenHeight;

            this->l_v2LastPosition  = this->l_fPosition();
            this->l_v2Position      = this->l_v2LastPosition;
            this->l_bFullScreen     = IsWindowFullscreen();
        }

        Vector2 l_v2MousePosition = GetMousePosition();
        if (this->F_bIsCollide(l_v2MousePosition))
        {
            this->l_bHover = true;
            this->l_v2Position.x = Lerp(this->l_v2Position.x, this->l_v2LastPosition.x + 20, 0.1);
        } else
        {
            this->l_v2Position.x = Lerp(this->l_v2Position.x, this->l_v2LastPosition.x, 0.1);
            this->l_bHover = false;
        }

        if (this->F_bIsPressed(l_v2MousePosition))
        {
            PlaySound(this->l_sSoundClick);
            this->l_bPressed = true;
            this->G_fAction();
        } else if (this->l_bPressed && IsMouseButtonUp(MOUSE_BUTTON_LEFT))
        {
            this->l_bPressed = false;
        }
    }

    void F_vDraw() override
    {
        if (this->l_bPressed) {
            //DrawText(this->l_sText.c_str(), (int) this->l_v2Position.x, (int) this->l_v2Position.y, (int) this->l_fSize, this->l_cColorPressed);
            DrawTextEx(this->l_fFont, this->l_sText.c_str(), this->l_v2Position, this->l_fSize, 1, this->l_cColorPressed);
        } else if (this->l_bHover)
        {
            //DrawText(this->l_sText.c_str(), (int) this->l_v2Position.x, (int) this->l_v2Position.y, (int) this->l_fSize, this->l_cColorHover);
            DrawTextEx(this->l_fFont, this->l_sText.c_str(), this->l_v2Position, this->l_fSize, 1, this->l_cColorHover);
        } else
        {
            //DrawText(this->l_sText.c_str(), (int) this->l_v2Position.x, (int) this->l_v2Position.y, (int) this->l_fSize, this->l_cColorDefault);
            DrawTextEx(this->l_fFont, this->l_sText.c_str(), this->l_v2Position, this->l_fSize, 1, this->l_cColorDefault);
        }
    }
};

class C_Slider : public C_Component
{
private:
    Color l_cColorDefault;
    Color l_cColorSlide;
    Color l_cColorButton;

    std::function<Vector2(void)>    l_fPosition;

    Vector2 l_v2Position;
    int l_iWidth;
    int l_iHeight;

    int l_iScreenWidth;
    int l_iScreenHeight;

    int l_iValue;
    int l_iLastValue;
    int l_iMaxValue;

    int  l_iFocus   = 0;
    int  l_iPressed = 0;

    Sound l_sSoundClick;
    Font  l_fFont;

    std::function<void(int, int)>    G_fAction;

public:

    explicit C_Slider(int l_iValue, int l_iMaxValue, std::function<Vector2(void)> l_fPosition, int l_iWidth, int l_iHeight, std::function<void(int, int)> l_fAction= nullptr, Color l_cColor=RAYWHITE)
            : l_iValue(l_iValue), l_iLastValue(l_iValue), l_iMaxValue(l_iMaxValue),
              l_fPosition(std::move(l_fPosition)),

              G_fAction(std::move(l_fAction)),

              l_iWidth(l_iWidth),
              l_iHeight(l_iHeight),

              l_cColorDefault(l_cColor),
              l_cColorSlide((Color) {static_cast<unsigned char>(l_cColor.r >> 1), static_cast<unsigned char>(l_cColor.g >> 1), static_cast<unsigned char>(l_cColor.b >> 1), 255}),
              l_cColorButton((Color) {static_cast<unsigned char>(l_cColor.r >> 2), static_cast<unsigned char>(l_cColor.g >> 2), static_cast<unsigned char>(l_cColor.b >> 2), 255})
    {
        this->l_v2Position      = this->l_fPosition();

        this->l_iScreenWidth    = GetScreenWidth();
        this->l_iScreenHeight   = GetScreenHeight();

        this->l_sSoundClick     = LoadSound(("./resources/assets/music/ui/Abstract1.wav"));
        this->l_fFont           = GetFontDefault();
    };


    bool F_bIsCollide(Vector2 l_v2Point)
    {
        return CheckCollisionPointRec((Vector2) l_v2Point, (Rectangle){ this->l_v2Position.x + this->l_iValue, this->l_v2Position.y, (float) l_iHeight/2, (float) l_iHeight });
    }

    bool F_bIsPressed(Vector2 l_v2Point)
    {
        if (this->F_bIsCollide(l_v2Point))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && this->l_iPressed == 1)
            {
                this->l_iPressed = 0;
                return false;
            }
        }

        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && this->l_iPressed == 0)
        {
            this->l_iPressed = 1;
            return this->F_bIsCollide(l_v2Point);
        }

        return false;
    }

    bool F_bIsDown(Vector2 l_v2Point) {
        if (this->F_bIsCollide(l_v2Point))
        {
            return IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        }
        return false;
    }


    void F_vUpdate() override
    {
        int l_iTempScreenWidth    = GetScreenWidth();
        int l_iTempScreenHeight   = GetScreenHeight();

        if (this->l_iScreenWidth != l_iTempScreenWidth || this->l_iScreenHeight != l_iTempScreenHeight)
        {
            this->l_iScreenWidth    = l_iTempScreenWidth;
            this->l_iScreenHeight   = l_iTempScreenHeight;

            this->l_v2Position  = this->l_fPosition();
        }

        Vector2 l_v2MousePosition = GetMousePosition();
        if (this->F_bIsDown(l_v2MousePosition) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && this->l_iFocus == 1))
        {
            this->l_iFocus = 1;
            int l_iTempValue = GetMouseX() - this->l_v2Position.x;

            if (l_iTempValue > this->l_iWidth)
            {
                l_iTempValue = this->l_iWidth;
            } else if (l_iTempValue < 0)
                l_iTempValue = 0;

            this->l_iValue = l_iTempValue;
        } else if (IsMouseButtonUp(MOUSE_BUTTON_LEFT) && this->l_iFocus == 1)
        {
            this->l_iFocus = 0;
        }

        if (this->F_bIsPressed(l_v2MousePosition))
            PlaySound(this->l_sSoundClick);

        if (this->l_iLastValue != this->l_iValue)
        {
            this->G_fAction(this->l_iValue, this->l_iMaxValue);
            this->l_iLastValue = this->l_iValue;
        }
    }

    void F_vDraw() override
    {
        DrawRectangleRounded((Rectangle) { this->l_v2Position.x, this->l_v2Position.y, (float) this->l_iWidth, (float) this->l_iHeight}, 0.5, 100, this->l_cColorDefault);
        DrawRectangleRounded((Rectangle) { this->l_v2Position.x, this->l_v2Position.y, (float) this->l_iValue, (float) this->l_iHeight}, 0.5, 100, this->l_cColorSlide);
        DrawRectangleRounded((Rectangle) { this->l_v2Position.x + this->l_iValue - (this->l_iHeight/2)/2, this->l_v2Position.y, (float) this->l_iHeight/2, (float) this->l_iHeight}, 0.5, 100, this->l_cColorButton);
    }
};

class C_Text : public C_Component
{
private:
    Color                           l_cColorDefault;

    std::string                     l_sText;
    float                           l_fSize;
    std::function<Vector2(void)>    l_fPosition;

    Vector2                         l_v2Position;

    int l_iScreenWidth;
    int l_iScreenHeight;

    Font  l_fFont;

public:

    explicit C_Text(std::string l_sText, float l_fSize, std::function<Vector2(void)> l_fPosition, Color l_cColor=RAYWHITE)
            : l_sText(std::move(l_sText)), l_fSize(l_fSize),
              l_fPosition(std::move(l_fPosition)),

              l_cColorDefault(l_cColor)
    {
        this->l_v2Position  = this->l_fPosition();

        this->l_iScreenWidth    = GetScreenWidth();
        this->l_iScreenHeight   = GetScreenHeight();

        this->l_fFont           = GetFontDefault();//LoadFont("./resources/assets/font/m3x6.ttf");
    };

    void F_vUpdate() override
    {
        int l_iTempScreenWidth    = GetScreenWidth();
        int l_iTempScreenHeight   = GetScreenHeight();

        if (this->l_iScreenWidth != l_iTempScreenWidth || this->l_iScreenHeight != l_iTempScreenHeight)
        {
            this->l_iScreenWidth    = l_iTempScreenWidth;
            this->l_iScreenHeight   = l_iTempScreenHeight;

            this->l_v2Position      = this->l_fPosition();
        }
    }

    void F_vDraw() override
    {
        DrawTextEx(this->l_fFont, this->l_sText.c_str(), this->l_v2Position, this->l_fSize, 1, this->l_cColorDefault);
    }
};

class C_CheckBox : public C_Component
{
private:
    Color l_cColorText;
    Color l_cColorDefault;
    Color l_cColorBorder;

    std::string                     l_sText;
    float                           l_fSize;
    std::function<Vector2(void)>    l_fPosition;

    Vector2 l_v2Position;

    int l_iScreenWidth;
    int l_iScreenHeight;

    int  l_iPressed = 1;
    bool l_bChecked = false;

    Sound l_sSoundClick;
    Font  l_fFont;

    std::function<void(bool)>    G_fAction;

public:

    explicit C_CheckBox(std::string l_sText, float l_fSize, std::function<Vector2(void)> l_fPosition, std::function<void(bool)> l_fAction= nullptr, bool l_bIsCheck=false, Color l_cColor=RAYWHITE, Color l_cColorText=RAYWHITE)
            : l_sText(std::move(l_sText)), l_fSize(l_fSize),
              l_fPosition(std::move(l_fPosition)),

              l_bChecked(l_bIsCheck),
              G_fAction(std::move(l_fAction)),

              l_cColorText(l_cColorText),
              l_cColorDefault(l_cColor),
              l_cColorBorder((Color) {static_cast<unsigned char>(l_cColor.r >> 1), static_cast<unsigned char>(l_cColor.g >> 1), static_cast<unsigned char>(l_cColor.b >> 1), 255})
    {
        this->l_v2Position      = this->l_fPosition();

        this->l_iScreenWidth    = GetScreenWidth();
        this->l_iScreenHeight   = GetScreenHeight();

        this->l_sSoundClick     = LoadSound(("./resources/assets/music/ui/Abstract1.wav"));
        this->l_fFont           = GetFontDefault();
    };

    bool F_bIsCollide(Vector2 l_v2Point)
    {
        Vector2 l_v2MeasureText = MeasureTextEx(GetFontDefault(), this->l_sText.c_str(), this->l_fSize, 1);
        return CheckCollisionPointRec((Vector2) l_v2Point, (Rectangle){ this->l_v2Position.x + l_v2MeasureText.x + 16, this->l_v2Position.y + 2, (float) l_v2MeasureText.y - 4, (float) l_v2MeasureText.y - 4 });
    }

    bool F_bIsPressed(Vector2 l_v2Point)
    {
        return this->F_bIsCollide(l_v2Point) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    void F_vUpdate() override
    {
        int l_iTempScreenWidth    = GetScreenWidth();
        int l_iTempScreenHeight   = GetScreenHeight();

        if (this->l_iScreenWidth != l_iTempScreenWidth || this->l_iScreenHeight != l_iTempScreenHeight)
        {
            this->l_iScreenWidth    = l_iTempScreenWidth;
            this->l_iScreenHeight   = l_iTempScreenHeight;

            this->l_v2Position      = this->l_fPosition();
        }

        Vector2 l_v2MousePosition = GetMousePosition();

        if (this->F_bIsPressed(l_v2MousePosition))
        {
            PlaySound(this->l_sSoundClick);
            this->l_bChecked = !this->l_bChecked;
            this->G_fAction(this->l_bChecked);
        }
    }

    void F_vDraw() override
    {
        DrawTextEx(this->l_fFont, this->l_sText.c_str(), this->l_v2Position, this->l_fSize, 1, this->l_cColorText);
        Vector2 l_v2MeasureText = MeasureTextEx(this->l_fFont, this->l_sText.c_str(), this->l_fSize, 1);
        DrawRectangleRounded((Rectangle) { this->l_v2Position.x + l_v2MeasureText.x + 14, this->l_v2Position.y, (float) l_v2MeasureText.y, (float) l_v2MeasureText.y}, 0.65, 100, this->l_cColorDefault);
        DrawRectangleRounded((Rectangle) { this->l_v2Position.x + l_v2MeasureText.x + 16, this->l_v2Position.y + 2, (float) l_v2MeasureText.y - 4, (float) l_v2MeasureText.y - 4}, 0.65, 100, this->l_cColorBorder);

        if (this->l_bChecked)
        {
            DrawTextEx(this->l_fFont, "*", (Vector2) { this->l_v2Position.x + l_v2MeasureText.x + 10 + (l_v2MeasureText.x/this->l_sText.size()), this->l_v2Position.y + 2 }, this->l_fSize, 1, this->l_cColorText);
        }
    }
};

class C_DropDown : public C_Component
{
private:
    Color l_cColorText;
    Color l_cColorDefault;
    Color l_cColorBorder;

    std::string                     l_sTitle;
    std::vector<std::string>        l_lTextElements;
    int                             l_iSelectElements = 0;

    std::string                     l_sText;
    float                           l_fSize;
    std::function<Vector2(void)>    l_fPosition;
    int l_iWidth;
    int l_iHeight;

    Vector2 l_v2Position;

    int l_iScreenWidth;
    int l_iScreenHeight;

    bool l_bHover       = false;
    bool l_bShowList    = false;

    Sound l_sSoundClick;
    Font  l_fFont;

    std::function<void(int, std::vector<std::string>)>    G_fAction;

public:

    explicit C_DropDown(std::string l_sText, std::string l_sTitle, std::vector<std::string> l_lTextElements, float l_fSize, std::function<Vector2(void)> l_fPosition, int l_iWidth, int l_iHeight, std::function<void(int, std::vector<std::string>)> l_fAction= nullptr, Color l_cColor=RAYWHITE, Color l_cColorText=RAYWHITE)
            : l_sText(std::move(l_sText)), l_sTitle(std::move(l_sTitle)), l_fSize(l_fSize),

              l_fPosition(std::move(l_fPosition)),
              l_iWidth(l_iWidth),
              l_iHeight(l_iHeight),

              l_lTextElements(std::move(l_lTextElements)),
              G_fAction(l_fAction),

              l_cColorText(l_cColorText),
              l_cColorDefault(l_cColor),
              l_cColorBorder((Color) {static_cast<unsigned char>(l_cColor.r >> 1), static_cast<unsigned char>(l_cColor.g >> 1), static_cast<unsigned char>(l_cColor.b >> 1), 255})
    {
        this->l_v2Position      = this->l_fPosition();

        this->l_iScreenWidth    = GetScreenWidth();
        this->l_iScreenHeight   = GetScreenHeight();

        this->l_sSoundClick     = LoadSound(("./resources/assets/music/ui/Abstract1.wav"));
        this->l_fFont           = GetFontDefault();
    };

    bool F_bIsCollide(Vector2 l_v2Point, Rectangle l_rRect)
    {
        return CheckCollisionPointRec((Vector2) l_v2Point, l_rRect);
    }

    bool F_bIsPressed(Vector2 l_v2Point)
    {
        Vector2 l_v2MeasureText = MeasureTextEx(GetFontDefault(), this->l_sText.c_str(), this->l_fSize, 1);
        Rectangle l_rRect = (Rectangle){ this->l_v2Position.x + l_v2MeasureText.x + 16, this->l_v2Position.y, (float) this->l_iWidth, (float) this->l_iHeight };
        return this->F_bIsCollide(l_v2Point, l_rRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    bool F_bIsPressedOut(Vector2 l_v2Point)
    {
        Vector2 l_v2MeasureText = MeasureTextEx(GetFontDefault(), this->l_sText.c_str(), this->l_fSize, 1);
        Rectangle l_rRect = (Rectangle){ this->l_v2Position.x + l_v2MeasureText.x + 16, this->l_v2Position.y, (float) this->l_iWidth, (float) this->l_iHeight };
        return !this->F_bIsCollide(l_v2Point, l_rRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    void F_vUpdate() override
    {
        int l_iTempScreenWidth    = GetScreenWidth();
        int l_iTempScreenHeight   = GetScreenHeight();

        if (this->l_iScreenWidth != l_iTempScreenWidth || this->l_iScreenHeight != l_iTempScreenHeight)
        {
            this->l_iScreenWidth    = l_iTempScreenWidth;
            this->l_iScreenHeight   = l_iTempScreenHeight;

            this->l_v2Position      = this->l_fPosition();
        }

        Vector2 l_v2MousePosition = GetMousePosition();

        if (this->F_bIsPressed(l_v2MousePosition))
        {
            PlaySound(this->l_sSoundClick);
            this->l_bShowList = !this->l_bShowList;
        }

        if (this->l_bShowList)
        {
            for (int l_iId = 0; l_iId < this->l_lTextElements.size(); ++l_iId)
            {
                Vector2 l_v2MeasureText = MeasureTextEx(GetFontDefault(), this->l_sText.c_str(), this->l_fSize, 1);
                Rectangle l_rRect = (Rectangle){ this->l_v2Position.x + l_v2MeasureText.x + 16, this->l_v2Position.y + 2 + (float) ((l_iId+1)*this->l_iHeight), (float) this->l_iWidth, (float) this->l_iHeight-4 };
                if(this->F_bIsCollide(l_v2MousePosition, l_rRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    PlaySound(this->l_sSoundClick);
                    this->l_iSelectElements = l_iId;
                    this->l_sTitle = l_lTextElements[this->l_iSelectElements];
                    this->G_fAction(this->l_iSelectElements, this->l_lTextElements);
                }
            }
        }

        if (this->F_bIsPressedOut(l_v2MousePosition))
        {
            this->l_bShowList = false;
        }
    }

    void F_vDraw() override
    {
        Vector2 l_v2MeasureText = MeasureTextEx(GetFontDefault(), this->l_sText.c_str(), this->l_fSize, 1);
        Rectangle l_rRect;
        Rectangle l_rRectBorder;

        if(this->l_bShowList) {
            l_rRectBorder           = (Rectangle){ this->l_v2Position.x + l_v2MeasureText.x + 16, this->l_v2Position.y, (float) this->l_iWidth, (float) this->l_iHeight + (this->l_lTextElements.size()*this->l_iHeight) };
            l_rRect                 = (Rectangle){ this->l_v2Position.x + l_v2MeasureText.x + 18, this->l_v2Position.y + 2, (float) this->l_iWidth - 4, (float) this->l_iHeight + (this->l_lTextElements.size()*this->l_iHeight) - 4 };
        } else
        {
            l_rRectBorder                 = (Rectangle){ this->l_v2Position.x + l_v2MeasureText.x + 16, this->l_v2Position.y, (float) this->l_iWidth, (float) this->l_iHeight };
            l_rRect                       = (Rectangle){ this->l_v2Position.x + l_v2MeasureText.x + 18, this->l_v2Position.y + 2, (float) this->l_iWidth - 4, (float) this->l_iHeight - 4 };
        }


        DrawRectangleRounded(l_rRectBorder, this->l_bShowList ? 0.1 : 0.5, 0, this->l_cColorDefault);
        DrawRectangleRounded(l_rRect, this->l_bShowList ? 0.1 : 0.5, 0, this->l_cColorBorder);
        DrawTextEx(this->l_fFont, this->l_sText.c_str(), this->l_v2Position, this->l_fSize, 1, this->l_cColorText);

        DrawTextEx(this->l_fFont, this->l_sTitle.c_str(), (Vector2) { this->l_v2Position.x + l_v2MeasureText.x + 18 + (this->l_fFont.recs->width/this->l_sTitle.size()), this->l_v2Position.y + 2 }, this->l_fSize, 1, this->l_cColorText);

        if (this->l_bShowList)
        {
            for (int l_iId = 0; l_iId < this->l_lTextElements.size(); ++l_iId)
            {
                DrawTextEx(this->l_fFont, this->l_lTextElements[l_iId].c_str(), (Vector2) { this->l_v2Position.x + l_v2MeasureText.x + 18 + (this->l_fFont.recs->width/this->l_sTitle.size()), this->l_v2Position.y + 2 + (float) ((l_iId+1)*this->l_iHeight) }, this->l_fSize, 1, this->l_cColorText);
            }
        }
    }
};

class C_TextField : public C_Component
{
public:
    enum STATE
    {
        STANDARD_TEXT = 0,
        SPECIFIC_TEXT,
        IP_TEXT,
        NUMERIC_TEXT,
        FLOAT_TEXT,
    };

private:
    Color G_cColorText;
    Color G_cColorDefault;
    Color G_cColorBorder;

    STATE                           G_eType;
    std::string                     G_sText;
    float                           G_fSize;
    std::function<Vector2(void)>    G_fPosition;
    int G_iWidth;
    int G_iHeight;

    Vector2 G_v2Position;

    int G_iScreenWidth;
    int G_iScreenHeight;

    bool G_bWriteText = false;

    int G_iCursorPosition;

    Sound G_sSoundClick;
    Font  G_fFont;

    std::function<void(std::string)>    G_fAction;

public:

    explicit C_TextField(C_TextField::STATE l_eType, std::string l_sText, float l_fSize, std::function<Vector2(void)> l_fPosition, int l_iWidth, int l_iHeight, std::function<void(std::string)> l_fAction= nullptr, Color l_cColor=RAYWHITE, Color l_cColorText=RAYWHITE)
            : G_eType(l_eType), G_sText(std::move(l_sText)), G_fSize(l_fSize),

              G_fPosition(std::move(l_fPosition)),
              G_iWidth(l_iWidth),
              G_iHeight(l_iHeight),

              G_fAction(std::move(l_fAction)),

              G_cColorText(l_cColorText),
              G_cColorDefault(l_cColor),
              G_cColorBorder((Color) {static_cast<unsigned char>(l_cColor.r >> 1), static_cast<unsigned char>(l_cColor.g >> 1), static_cast<unsigned char>(l_cColor.b >> 1), 255})
    {
        this->G_v2Position      = this->G_fPosition();

        this->G_iScreenWidth    = GetScreenWidth();
        this->G_iScreenHeight   = GetScreenHeight();

        this->G_sSoundClick     = LoadSound(("./resources/assets/music/ui/Abstract1.wav"));
        this->G_fFont           = GetFontDefault();

        this->G_iCursorPosition = this->G_sText.size()-1;
    };

    bool F_bIsCollide(Vector2 l_v2Point, Rectangle l_rRect)
    {
        return CheckCollisionPointRec((Vector2) l_v2Point, l_rRect);
    }

    bool F_bIsPressed(Vector2 l_v2Point)
    {
        Rectangle l_rRect = (Rectangle){ this->G_v2Position.x, this->G_v2Position.y, (float) this->G_iWidth, (float) this->G_iHeight };
        return this->F_bIsCollide(l_v2Point, l_rRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    bool F_bIsPressedOut(Vector2 l_v2Point)
    {
        Rectangle l_rRect = (Rectangle){ this->G_v2Position.x, this->G_v2Position.y, (float) this->G_iWidth, (float) this->G_iHeight };
        return !this->F_bIsCollide(l_v2Point, l_rRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    bool F_bIsAnyKeyPressed(int key)
    {
        bool keyPressed = false;

        switch (G_eType)
        {

            case STANDARD_TEXT:
                if ((key >= 32 && key <= 126)) keyPressed = true;
                break;
            case SPECIFIC_TEXT:
                if ((key >= 65 && key <= 90) || (key >= 97 && key <= 122) || (key == 20)) keyPressed = true;
                break;
            case IP_TEXT:
                if ((key >= 48 && key <= 57) || key == 46) keyPressed = true;
                break;
            case NUMERIC_TEXT:
                if ((key >= 48 && key <= 57)) keyPressed = true;
                break;
            case FLOAT_TEXT:
                if ((key >= 48 && key <= 57) || key == 46) keyPressed = true;
                break;
        }

        return keyPressed;
    }

    void F_vUpdate() override
    {
        int l_iTempScreenWidth    = GetScreenWidth();
        int l_iTempScreenHeight   = GetScreenHeight();

        if (this->G_iScreenWidth != l_iTempScreenWidth || this->G_iScreenHeight != l_iTempScreenHeight)
        {
            this->G_iScreenWidth    = l_iTempScreenWidth;
            this->G_iScreenHeight   = l_iTempScreenHeight;

            this->G_v2Position      = this->G_fPosition();
        }

        Vector2 l_v2MousePosition = GetMousePosition();

        if (this->F_bIsPressed(l_v2MousePosition))
        {
            PlaySound(this->G_sSoundClick);
            this->G_bWriteText = !this->G_bWriteText;
        }

        if (this->G_bWriteText)
        {
            int l_iKey                = GetCharPressed();
            int l_iActionKey          = GetKeyPressed();

            if(this->F_bIsAnyKeyPressed(l_iKey))
            {
                this->G_sText += (char) l_iKey;
                this->G_iCursorPosition++;

                this->G_fAction(G_sText);
            }

            if(l_iActionKey == KEY_BACKSPACE && this->G_sText.size() > 0)
            {
                this->G_sText.erase(this->G_iCursorPosition);
                this->G_iCursorPosition--;
                this->G_fAction(G_sText);
            }
        }

        if (this->F_bIsPressedOut(l_v2MousePosition) || IsKeyPressed(KEY_ENTER))
        {
            this->G_bWriteText = false;
        }
    }

    void F_vDraw() override
    {
        Rectangle l_rRect       = (Rectangle){ this->G_v2Position.x + 2, this->G_v2Position.y + 2, (float) this->G_iWidth - 4, (float) this->G_iHeight - 4 };
        Rectangle l_rRectBorder = (Rectangle){ this->G_v2Position.x, this->G_v2Position.y, (float) this->G_iWidth, (float) this->G_iHeight };

        DrawRectangleRounded(l_rRectBorder, 0.5, 0, this->G_cColorDefault);
        DrawRectangleRounded(l_rRect, 0.5, 0, this->G_cColorBorder);
        DrawTextEx(this->G_fFont, this->G_sText.c_str(), (Vector2) { this->G_v2Position.x + 2, static_cast<float>(this->G_v2Position.y + (this->G_fFont.recs->height*(0.1*this->G_fSize))/4) }, this->G_fSize, 1, this->G_cColorText);

        //DrawTextEx(this->l_fFont, this->l_sText.c_str(), (Vector2) { this->l_v2Position.x + l_v2MeasureText.x + 18 + (this->l_fFont.recs->width/this->l_sText.size()), this->l_v2Position.y + 2 }, this->l_fSize, 1, this->l_cColorText);
    }
};


#endif
#include "VerCore/IReflectable.hpp"
#include "VerCore/SpriteGroup.hpp"
UFILE()

#include "SCENE.h"

//****************************
//      Classic Include Components (do not touch)
UCLASS(C_Transform)
UCLASS(C_Camera)

//****************************
//      Your Include Components
#include "Player.hpp"
#include "Enemie.hpp"
#include "Network.hpp"

UCLASS(Player)
UCLASS(Enemie)
UCLASS(Network)
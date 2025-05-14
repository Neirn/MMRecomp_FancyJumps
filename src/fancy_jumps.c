#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "rand.h"

typedef enum
{
    MOD_OPT_VANILLA,
    MOD_OPT_CYCLE,
    MOD_OPT_RANDOM,
    MOD_OPT_DISABLED
} FancyJumpsSelectOption;

extern FloorProperty sPrevFloorProperty;

u8 currentJump = 0;

FloorProperty realPrevFloorProperty = FLOOR_PROPERTY_0;

u8 getNextJump()
{
    u8 result = 0;
    FancyJumpsSelectOption selectionMethod = recomp_get_config_u32("jump_selection_method");
    switch (selectionMethod)
    {
    case MOD_OPT_CYCLE:
        result = currentJump % 3;
        ++currentJump;
        break;

    case MOD_OPT_RANDOM:
        result = Rand_Next() % 3;
        break;

    default:
        break;
    }

    return result;
}

RECOMP_HOOK("func_808373F8")
void replaceFloorProperty(PlayState *play, Player *this, u16 sfxId)
{
    realPrevFloorProperty = sPrevFloorProperty;

    FancyJumpsSelectOption selectionMethod = recomp_get_config_u32("jump_selection_method");

    if (selectionMethod == MOD_OPT_DISABLED) {
        sPrevFloorProperty = FLOOR_PROPERTY_0;
        return;
    }

    if ((this->transformation != PLAYER_FORM_DEKU))
    {
        u8 selectedJump = getNextJump();

        switch (sPrevFloorProperty)
        {
        case FLOOR_PROPERTY_1:
        case FLOOR_PROPERTY_2:
            // do nothing
            break;

        default:
            switch (selectedJump)
            {
            case 1:
                sPrevFloorProperty = FLOOR_PROPERTY_1;
                break;

            case 2:
                sPrevFloorProperty = FLOOR_PROPERTY_2;
                break;

            default:
                break;
            }
            break;
        }
    }
}

RECOMP_HOOK_RETURN("func_808373F8")
void restoreFloorProperty()
{
    sPrevFloorProperty = realPrevFloorProperty;
}

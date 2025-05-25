#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "rand.h"
#include "z64animation.h"

extern FloorProperty sPrevFloorProperty;
extern LinkAnimationHeader gPlayerAnim_link_normal_newroll_jump_20f;
extern LinkAnimationHeader gPlayerAnim_link_normal_newside_jump_20f;

typedef enum {
    MOD_OPT_VANILLA,
    MOD_OPT_CYCLE,
    MOD_OPT_RANDOM,
    MOD_OPT_DISABLED,
} FancyJumpsSelectOption;

typedef enum {
    MOD_OPT_VANILLA_RESTRICT,
    MOD_OPT_UNRESTRICT,
} FancyJumpsUnrestrictOption;

u8 currentJump = 0;

FloorProperty realPrevFloorProperty = FLOOR_PROPERTY_0;

Player *player;

PlayState *playState;

FancyJumpsSelectOption selectionMethod;

u8 getNextJump() {
    u8 result = 0;

    switch (selectionMethod) {
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
void pre_OnStartJump(PlayState *play, Player *this, u16 sfxId) {
    player = this;
    playState = play;
    realPrevFloorProperty = sPrevFloorProperty;
    selectionMethod = recomp_get_config_u32("jump_selection_method");

    if (selectionMethod == MOD_OPT_DISABLED) {
        sPrevFloorProperty = FLOOR_PROPERTY_0;
        return;
    }

    if ((this->transformation != PLAYER_FORM_DEKU)) {
        u8 selectedJump = getNextJump();

        switch (sPrevFloorProperty) {
        case FLOOR_PROPERTY_1:
        case FLOOR_PROPERTY_2:
            // do nothing
            break;

        default:
            switch (selectedJump) {
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

bool isDoingFancyJump(Player *p) {
    return p->skelAnime.animation == &gPlayerAnim_link_normal_newside_jump_20f ||
           p->skelAnime.animation == &gPlayerAnim_link_normal_newroll_jump_20f;
}

RECOMP_HOOK_RETURN("func_808373F8")
void post_OnStartJump() {
    sPrevFloorProperty = realPrevFloorProperty;
    if (isDoingFancyJump(player) && recomp_get_config_u32("unrestrict_fancy") == MOD_OPT_UNRESTRICT) {
        player->stateFlags2 &= ~PLAYER_STATE2_80000;
    }
}

bool shouldRollAfterJump(Player *this) {
    return (this->fallDistance > 80) && (this->fallDistance < 800) &&
           (this->controlStickDirections[this->controlStickDataIndex] == PLAYER_STICK_DIR_FORWARD) &&
           !(this->stateFlags1 & PLAYER_STATE1_CARRYING_ACTOR);
}

RECOMP_HOOK("func_80837134")
void pre_OnJumpEnd(PlayState *play, Player *this) {
    if (isDoingFancyJump(this) && recomp_get_config_u32("unrestrict_fancy") == MOD_OPT_UNRESTRICT) {
        if (shouldRollAfterJump(this)) {
            player->stateFlags2 &= ~PLAYER_STATE2_80000;
        } else {
            player->stateFlags2 |= PLAYER_STATE2_80000;
        }
    }
}

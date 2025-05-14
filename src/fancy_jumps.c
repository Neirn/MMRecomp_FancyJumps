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
    MOD_OPT_DISABLED
} FancyJumpsSelectOption;

typedef enum {
    MOD_OPT_ALWAYS_ALLOW_ROLL,
    MOD_OPT_VANILLA_ALLOW_ROLL
} FancyJumpsAllowRollOption;

u8 currentJump = 0;

FloorProperty realPrevFloorProperty = FLOOR_PROPERTY_0;

u8 getNextJump() {
    u8 result = 0;
    FancyJumpsSelectOption selectionMethod = recomp_get_config_u32("jump_selection_method");
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

Player *player;
PlayState *playState;

RECOMP_HOOK("func_808373F8")
void replaceFloorProperty(PlayState *play, Player *this, u16 sfxId) {
    player = this;
    playState = play;
    realPrevFloorProperty = sPrevFloorProperty;

    FancyJumpsSelectOption selectionMethod = recomp_get_config_u32("jump_selection_method");

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

RECOMP_HOOK_RETURN("func_808373F8")
void restoreFloorProperty() {
    sPrevFloorProperty = realPrevFloorProperty;
}

bool shouldRollAfterJump(Player *this) {
    return (this->fallDistance > 80) && (this->fallDistance < 800) &&
           (this->controlStickDirections[this->controlStickDataIndex] == PLAYER_STICK_DIR_FORWARD) &&
           !(this->stateFlags1 & PLAYER_STATE1_CARRYING_ACTOR);
}

RECOMP_HOOK("func_80837134")
void onJumpEnd(PlayState *play, Player *this) {

    if (recomp_get_config_u32("allow_roll_after_fancy_jump") == MOD_OPT_ALWAYS_ALLOW_ROLL) {
        if (this->transformation != PLAYER_FORM_DEKU) {
            LinkAnimationHeader *animation = this->skelAnime.animation;
            if (animation == &gPlayerAnim_link_normal_newside_jump_20f || animation == &gPlayerAnim_link_normal_newside_jump_20f) {
                if (shouldRollAfterJump(this)) {
                    player->stateFlags2 &= ~PLAYER_STATE2_80000;
                }
            }
        }
    }
}

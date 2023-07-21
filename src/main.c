#include <gbdk/platform.h>
#include <gbdk/incbin.h>

#include <gb/isr.h>

#include "hUGEDriver.h"
#include "queues.h"

INCBIN_EXTERN(upper_picture_tiles)
INCBIN_EXTERN(upper_picture_map)
INCBIN_EXTERN(upper_picture_attr)

INCBIN_EXTERN(lower_picture_tiles)
INCBIN_EXTERN(lower_picture_map)
INCBIN_EXTERN(lower_picture_attr)

INCBIN_EXTERN(picture_palette)

INCBIN_EXTERN(font_tiles)

static uint16_t SP_SAVE;
void load_palettes(void) NAKED {
__asm
        push af
        push hl
        push bc
        push de

        ld (_SP_SAVE), sp           ; save SP

        ld de, #_picture_palette    ; load picture palettes address
        ldh a, (_SCY_REG)
        swap a
        ld l, a
        and #0x0f
        ld h, a
        ld a, #0xf0
        and l
        ld l, a
        add hl, hl
        add hl, de                  ; offset address by SCY * (4 * 4 * 2)
        ld sp, hl

        rlca                        ; compensate odd/even line
        and #0x20                   ; if odd then start from 4-th palette; offset == 32
        or  #0x80                   ; set auto-increment

        ld hl,#_BCPS_REG
        ld (hl+), a

        .rept (8*4)                 ; read and set the the colors that come from previous lines
            pop de
            ld (hl), e
            ld (hl), d
        .endm

2$:     ldh a, (_STAT_REG)
        and #STATF_BUSY
        jr z, 2$                    ; wait for mode 3

        ld a, #STATF_MODE00
        ld (_STAT_REG), a
        xor a
        ld (_IF_REG), a

4$:
        pop de                      ; preload the first two colors
        pop bc

        xor a
        ld (_IF_REG), a
        halt                        ; wait for mode 0

        ld (hl), e                  ; set the first two colors
        ld (hl), d
        ld (hl), c
        ld (hl), b

        .rept (4*4)-2
            pop de                  ; read and set the rest of the colors
            ld (hl), e
            ld (hl), d
        .endm

        ldh a, (_LY_REG)
        cp #143
        jr c, 4$                    ; load the next 4 palettes

        ld a, #STATF_LYC
        ld (_STAT_REG), a
        xor a
        ld (_IF_REG), a

        ld hl, #_SP_SAVE            ; restore SP
        ld a, (hl+)
        ld h, (hl)
        ld l, a
        ld sp, hl

        pop de
        pop bc
        pop hl
        pop af
        reti
__endasm;
}
ISR_VECTOR(VECTOR_STAT, load_palettes)

extern const hUGESong_t unreal_superhero2;

const unsigned int sprite_palettes[] = {
    RGB_BLACK, RGB_LIGHTGRAY,   RGB_BLACK,    RGB_WHITE,
    RGB_BLACK, RGB(15,  0,  0), RGB_BLACK,    RGB_RED,
    RGB_BLACK, RGB( 0, 15,  0), RGB_BLACK,    RGB_GREEN,
    RGB_BLACK, RGB( 0,  0, 15), RGB_BLACK,    RGB_BLUE,
    RGB_BLACK, RGB( 9,  0,  0), RGB_BLACK,    RGB_DARKRED,
    RGB_BLACK, RGB(15, 15,  0), RGB_BLACK,    RGB_YELLOW,
    RGB_BLACK, RGB( 0, 15, 15), RGB_BLACK,    RGB_CYAN,
    RGB_BLACK, RGB( 9,  0,  9), RGB_BLACK,    RGB_PURPLE
};

const uint8_t sin_table[] = {
     84, 85, 87, 89, 91, 93, 94, 96, 98,100,101,103,105,106,108,110,
    111,113,115,116,118,119,121,122,123,125,126,127,128,130,131,132,
    133,134,135,136,137,137,138,139,140,140,141,141,142,142,143,143,
    143,143,143,143,144,143,143,143,143,143,143,142,142,141,141,140,
    140,139,138,137,137,136,135,134,133,132,131,130,128,127,126,125,
    123,122,121,119,118,116,115,113,111,110,108,106,105,103,101,100,
     98, 96, 94, 93, 91, 89, 87, 85, 84, 83, 81, 79, 77, 75, 74, 72,
     70, 68, 67, 65, 63, 62, 60, 58, 57, 55, 53, 52, 50, 49, 47, 46,
     45, 43, 42, 41, 40, 38, 37, 36, 35, 34, 33, 32, 31, 31, 30, 29,
     28, 28, 27, 27, 26, 26, 25, 25, 25, 25, 25, 25, 24, 25, 25, 25,
     25, 25, 25, 26, 26, 27, 27, 28, 28, 29
};

const uint8_t text[] = "HELLO, WORLD! :)  THIS SMALL TECH DEMO WAS WRITTEN WITH GBDK-2020! " \
                       "THIS EXCELLENT \"UNREAL SUPERHERO\" MUSIC COVER WAS WRITTEN BY KABCORP. " \
                       "HUGETRACKER SOUND DRIVER BY SUPERDISK. " \
                       "WALLPAPER WITH THE CHARACTER FROM \"ALIEN HOMINID\" WAS DRAWN BY THE UNKNOWN AUTHOR. " \
                       "PRESS D-PAD TO SCROLL THIS PICTURE UP AND DOWN. IT IS A 2320-COLOR MULTICOLOR IMAGE. " \
                       "ENJOY. THANK YOU FOR READING THIS. TOXA.            ";
const uint8_t * text_ptr = text;

// main funxction
_Noreturn void main(void) {
    cpu_fast();

    wait_vbl_done();
    DISPLAY_OFF;

    for (uint8_t i = 1; i < 21; i++) free_id(i);

    NR52_REG = 0x80;
    NR51_REG = 0xFF;
    NR50_REG = 0x77;
    CRITICAL {
        hUGE_init(&unreal_superhero2);
        add_VBL(hUGE_dosound);

        LYC_REG = 152;
        STAT_REG = STATF_LYC;
        set_interrupts(IE_REG | VBL_IFLAG | LCD_IFLAG);
    }
    // load sprite palettes;
    set_sprite_palette(0, 8, sprite_palettes);

    // load font tiles
    set_sprite_data(0x20, INCBIN_SIZE(font_tiles) >> 4, font_tiles);

    // load picture tiles and map
    set_bkg_data(0, INCBIN_SIZE(upper_picture_tiles) >> 4, upper_picture_tiles);
    set_bkg_tiles(0, 0, 20, 11, upper_picture_map);
    VBK_REG = VBK_BANK_1;
    set_bkg_tiles(0, 0, 20, 11, upper_picture_attr);
    set_bkg_data(0, INCBIN_SIZE(lower_picture_tiles) >> 4, lower_picture_tiles);
    set_bkg_tiles(0, 11, 20, 10, lower_picture_attr);
    VBK_REG = VBK_BANK_0;
    set_bkg_tiles(0, 11, 20, 10, lower_picture_map);

    SHOW_BKG;
    SHOW_SPRITES;

    wait_vbl_done();
    DISPLAY_ON;

    static uint8_t tick = 0;

    while(TRUE) {
        // allocate sprites for letters
        if ((tick++ & 0x0f) == 0) {

            uint8_t ch;
            switch (ch = *text_ptr++) {
                case 0:
                    text_ptr = text;
                    break;
                case ' ':
                    break;
                default: {
                    uint8_t id;
                    if (id = alloc_id()) {
                        OAM_item_t * OAM_item = shadow_OAM + add_to_display(id - 1);
                        OAM_item->tile = ch;
                        OAM_item->y = sin_table[168];
                        OAM_item->x = 168;
                        OAM_item->prop = 0;
                    }
                    break;
                }
            }
        }

        // move scroll
        if (display_head != display_tail) {
            static uint8_t i;
            for (i = display_tail; i != display_head; ) {
                OAM_item_t * OAM_item = shadow_OAM + display[i = (i + 1) & QUEUE_MASK];
                if (--OAM_item->x) {
                    OAM_item->y = sin_table[OAM_item->x];
                } else {
                    OAM_item->y = 0;
                    free_id(remove_from_display());
                }
            }
        }

        // control scroll
        uint8_t joy = joypad();
        if (joy & J_UP) {
            if (SCY_REG) SCY_REG--;
        } else if (joy & J_DOWN) {
            if (SCY_REG < (uint8_t)(((11 + 10) * 8) - 144)) SCY_REG++;
        }

        wait_vbl_done();
    }
}

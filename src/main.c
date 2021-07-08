#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/isr.h>
#include <gb/incbin.h>

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

void load_palettes() __naked {
__asm
        push af
        push hl
        push bc
        push de

        ldhl sp, #0
        ld  b, h
        ld  c, l

        ld  de, #_picture_palette  ; load picture palettes address
        ldh a, (_SCY_REG)
        ld l, a
        ld h, #0
        add hl, hl
        add hl, hl
        add hl, hl
        add hl, hl
        add hl, hl
        add hl, de                 ; offset address by SCY * (4 * 4 * 2)
        ld  sp, hl

        rra
        ld  a, #0x80               ; auto-increment, start from 0 
        jr  nc, 0$
        set 5, a                   ; auto-increment start from 4-th palette; offset == 32
0$:

        ld  hl,#_BCPS_REG
        ld  (hl+), a

        .rept (8*4)             ; read and set the the colors that come from previous lines
            pop de              
            ld (hl), e
            ld (hl), d
        .endm

        ld  a, l
        ld  l, #<_STAT_REG
2$:     bit 1, (hl)
        jr  z, 2$
        ld  l, a

4$:
        pop de                  ; read first color of the first palette

        ld  a, l
        ld  l, #<_STAT_REG
3$:
        bit 1, (hl)
        jr  nz, 3$ 
        ld  l, a

        ld (hl), e              ; set the first color
        ld (hl), d

        .rept (4*4)-1
            pop de              ; read and set the rest of the colors
            ld (hl), e
            ld (hl), d
        .endm

        ldh a, (_LY_REG)
        cp  #143
        jr  c, 4$

        ld  h, b
        ld  l, c
        ld  sp, hl

        pop de
        pop bc
        pop hl
        pop af
        reti
__endasm;
}
ISR_VECTOR(VECTOR_STAT, load_palettes)

extern const hUGESong_t Intro;

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
    84,85,87,89,91,93,94,96,98,100,101,103,105,106,108,110,111,113,115,116,118,119,121,122,123,125,126,127,
    128,130,131,132,133,134,135,136,137,137,138,139,140,140,141,141,142,142,143,143,143,143,143,143,144,143,
    143,143,143,143,143,142,142,141,141,140,140,139,138,137,137,136,135,134,133,132,131,130,128,127,126,125,
    123,122,121,119,118,116,115,113,111,110,108,106,105,103,101,100,98,96,94,93,91,89,87,85,84,83,81,79,77,
    75,74,72,70,68,67,65,63,62,60,58,57,55,53,52,50,49,47,46,45,43,42,41,40,38,37,36,35,34,33,32,31,31,30,
    29,28,28,27,27,26,26,25,25,25,25,25,25,24,25,25,25,25,25,25,26,26,27,27,28,28,29
};

const char text[] = "HELLO, WORLD! :)  THIS SMALL TECH DEMO WAS WRITTEN WITH GBDK-2020! " \
					"THIS EXCELLENT \"UNREAL SUPERHERO\" MUSIC COVER WAS MADE BY KABCORP. " \
					"HUGETRACKER SOUND DRIVER BY SUPERDISK. " \
					"WALLPAPER FROM UNKNOWN AUTHOR. " \
					"PRESS D-PAD TO SCROLL THIS PICTURE UP AND DOWN. IT IS A 2560-COLOR MULTICOLOR IMAGE. " \
					"ENJOY. THANK YOU FOR READING THIS. TOXA.            ";
const char * text_ptr = text;

// main funxction
void main(void) {  
    cpu_fast();

    wait_vbl_done();
    DISPLAY_OFF;

    for (uint8_t i = 1; i < 21; i++) free_id(i);

    NR52_REG = 0x80;
    NR51_REG = 0xFF;
    NR50_REG = 0x77;
    __critical {
        hUGE_init(&Intro);
        add_VBL(hUGE_dosound);

        LYC_REG = 152;
        STAT_REG = 0b01000000;
        set_interrupts(VBL_IFLAG | LCD_IFLAG);
    }
    // load sprite palettes;
    set_sprite_palette(0, 8, sprite_palettes);

    // load font tiles
    set_sprite_data(0x20, SIZE(font_tiles) >> 4, font_tiles);

    // load picture tiles and map
    set_bkg_data(0, SIZE(upper_picture_tiles) >> 4, upper_picture_tiles);
    set_bkg_tiles(0, 0, 20, 11, upper_picture_map);
    VBK_REG = 1;
    set_bkg_tiles(0, 0, 20, 11, upper_picture_attr);
    set_bkg_data(0, SIZE(lower_picture_tiles) >> 4, lower_picture_tiles);
    set_bkg_tiles(0, 11, 20, 10, lower_picture_attr);
    VBK_REG = 0;
    set_bkg_tiles(0, 11, 20, 10, lower_picture_map);

    SHOW_BKG;
    SHOW_SPRITES;

    wait_vbl_done();
    DISPLAY_ON;

    static uint8_t tick;
    tick = 0;

    while(1) {    
        tick++;
        if (tick == 20) {
            tick = 0;
            static char ch;
            ch = *text_ptr++;
            if (ch != ' ') {
                uint8_t id = alloc_id();
                if (id != 0) {
                    OAM_item_t * OAM_item = shadow_OAM + id;
                    OAM_item->y = sin_table[168];
                    OAM_item->x = 168;
                    OAM_item->tile = ch;
                    OAM_item->prop = 0;
                    add_to_display(id);
                }
            }
            if (*text_ptr == 0) text_ptr = text;
        }

        // move scroll
        if (display_head != display_tail) {
            static uint8_t i;
            for (i = display_tail; i != display_head; ) {
                i++; i &= QUEUE_MASK;
                uint8_t id = display[i];
                uint8_t cx = --shadow_OAM[id].x;
                if (cx == 0) {
                    free_id(remove_from_display()); 
                    shadow_OAM[id].y = 0; 
                } else {
                    shadow_OAM[id].y = sin_table[cx];
                }
            }
        }

        uint8_t joy = joypad();
        if (joy & J_UP) {
            if (SCY_REG) SCY_REG--;
        } else if (joy & J_DOWN) {
            if (SCY_REG < (uint8_t)(((11 + 10) * 8) - 144)) SCY_REG++;
        }    
        wait_vbl_done();
    }
}

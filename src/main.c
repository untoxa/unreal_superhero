#include <gbdk/platform.h>
#include <gbdk/incbin.h>

#include <gb/isr.h>

#include "hUGEDriver.h"
#include "gbc_hicolor.h"
#include "queues.h"

#include "unreal_background.h"
#include "font8x8.h"

INCBIN_EXTERN(font_tiles)

#define MIN(A,B) ((A)<(B)?(A):(B))

static uint16_t SP_SAVE;
static uint8_t * p_hicolor_palettes;
static uint8_t p_hicolor_height;
void load_palettes(void) NAKED {
__asm
        push af
        push hl
        push bc
        push de

        ld (_SP_SAVE), sp           ; save SP

        ld hl, #_p_hicolor_palettes ; load address of picture palettes buffer
        ld a, (hl+)
        ld d, (hl)
        ld e, a

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
        or  #BCPSF_AUTOINC          ; set auto-increment

        ld hl, #_BCPS_REG
        ld (hl+), a                 ; HL now points to BCPD

        ld c, #(8 * 4)              ; read and set the the colors that come from previous lines
2$:
        pop de
        ld (hl), e
        ld (hl), d
        dec c
        jr nz, 2$

0$:
        ldh a, (_STAT_REG)
        and #STATF_BUSY
        jr z, 0$                    ; wait for mode 3

        ld a, #STATF_MODE00
        ldh (_STAT_REG), a

1$:
        pop bc                      ; preload the first two colors
        pop de

        xor a
        ldh (_IF_REG), a
        halt                        ; wait for mode 0

        ld (hl), c                  ; set the first two colors
        ld (hl), b
        ld (hl), e
        ld (hl), d

        .rept (4*4)-2               ; read and set four palettes except the two previously set colors
            pop de
            ld (hl), e
            ld (hl), d
        .endm

        ld a, (_p_hicolor_height)
        ld c, a
        ldh a, (_LY_REG)
        cp c
        jr c, 1$                    ; load the next 4 palettes

        ld a, #STATF_LYC
        ldh (_STAT_REG), a
        xor a
        ldh (_IF_REG), a

        ld sp, #_SP_SAVE
        pop hl
        ld sp, hl                   ; restore SP

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
    RGB_WHITE  , RGB_LIGHTGRAY,   RGB_BLACK,    RGB_BLACK,
    RGB_RED    , RGB(15,  0,  0), RGB_BLACK,    RGB_BLACK,
    RGB_GREEN  , RGB( 0, 15,  0), RGB_BLACK,    RGB_BLACK,
    RGB_BLUE   , RGB( 0,  0, 15), RGB_BLACK,    RGB_BLACK,
    RGB_DARKRED, RGB( 9,  0,  0), RGB_BLACK,    RGB_BLACK,
    RGB_YELLOW , RGB(15, 15,  0), RGB_BLACK,    RGB_BLACK,
    RGB_CYAN   , RGB( 0, 15, 15), RGB_BLACK,    RGB_BLACK,
    RGB_PURPLE , RGB( 9,  0,  9), RGB_BLACK,    RGB_BLACK
};

#define SPRITE_START_POSITION (DEVICE_SPRITE_PX_OFFSET_X + DEVICE_SCREEN_PX_WIDTH)

const uint8_t sin_table[SPRITE_START_POSITION + 1] = {
    84,  85,  87,  89,  91,  93,  94,  96,  98,  100, 101, 103, 105, 106, 108, 110,
    111, 113, 115, 116, 118, 119, 121, 122, 123, 125, 126, 127, 128, 130, 131, 132,
    133, 134, 135, 136, 137, 137, 138, 139, 140, 140, 141, 141, 142, 142, 143, 143,
    143, 143, 143, 143, 144, 143, 143, 143, 143, 143, 143, 142, 142, 141, 141, 140,
    140, 139, 138, 137, 137, 136, 135, 134, 133, 132, 131, 130, 128, 127, 126, 125,
    123, 122, 121, 119, 118, 116, 115, 113, 111, 110, 108, 106, 105, 103, 101, 100,
    98,  96,  94,  93,  91,  89,  87,  85,  84,  83,  81,  79,  77,  75,  74,  72,
    70,  68,  67,  65,  63,  62,  60,  58,  57,  55,  53,  52,  50,  49,  47,  46,
    45,  43,  42,  41,  40,  38,  37,  36,  35,  34,  33,  32,  31,  31,  30,  29,
    28,  28,  27,  27,  26,  26,  25,  25,  25,  25,  25,  25,  24,  25,  25,  25,
    25,  25,  25,  26,  26,  27,  27,  28,  28
};

const uint8_t text[] = "Hello, world! :)  This small tech demo was written using GBDK-2020! " \
                       "This excellent \"UNREAL SUPERHERO\" music cover was written by KABCORP. " \
                       "hUGETracker sound driver by SUPERDISK. " \
                       "Font by DamienG. " \
                       "Wallpaper with the character from the \"ALIEN HOMINID\" was drawn by the unknown author. " \
                       "Press D-PAD to scroll this picture UP and DOWN. This is a 2320-color multicolor image. " \
                       "Enjoy. Thank you for reading this. TOXA.            ";
const uint8_t * text_ptr = text;

uint8_t scroll_limit;

inline void load_image_data(const hicolor_data * p_hicolor){
    p_hicolor_palettes = p_hicolor->p_palette;
    p_hicolor_height = (p_hicolor->height_in_tiles > DEVICE_SCREEN_HEIGHT) ? (DEVICE_SCREEN_PX_HEIGHT - 1) : ((p_hicolor->height_in_tiles << 3) - 1);
    VBK_REG = VBK_BANK_0;
    set_bkg_data(0u, MIN(p_hicolor->tile_count, 256), p_hicolor->p_tiles);
    set_bkg_tiles(0u, 0u, DEVICE_SCREEN_WIDTH, p_hicolor->height_in_tiles, p_hicolor->p_map);
    VBK_REG = VBK_BANK_1;
    if (p_hicolor->tile_count > 256) set_bkg_data(0u, (p_hicolor->tile_count - 256), p_hicolor->p_tiles + (256 * 16));
    set_bkg_tiles(0, 0, DEVICE_SCREEN_WIDTH, p_hicolor->height_in_tiles, p_hicolor->p_attribute_map);
    VBK_REG = VBK_BANK_0;

    scroll_limit = ((p_hicolor->height_in_tiles * 8u) > DEVICE_SCREEN_PX_HEIGHT) ? ((p_hicolor->height_in_tiles * 8u) - DEVICE_SCREEN_PX_HEIGHT) : 0;
}

// main funxction
NORETURN void main(void) {
    cpu_fast();

    vsync();
    DISPLAY_OFF;

    // algo can allocate up to QUEUE_LEN-1 sprites
    for (uint8_t i = 1; i != QUEUE_LEN; i++) free_id(i);

    NR52_REG = AUDENA_ON;
    NR51_REG = 0xFF;
    NR50_REG = 0x77;
    CRITICAL {
        hUGE_init(&unreal_superhero2);
        add_VBL(hUGE_dosound);

        LYC_REG = 152;
        STAT_REG = STATF_LYC;
    }
    set_interrupts(IE_REG | LCD_IFLAG);
    // load sprite palettes;
    set_sprite_palette(0, 8, sprite_palettes);

    // load font tiles
    set_sprite_data(0x20, font8x8_TILE_COUNT, font8x8_tiles);

    // Copy address of palette into local var used by HiColor ISR
    load_image_data(&unreal_background_data);

    SHOW_BKG;
    SHOW_SPRITES;

    vsync();
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
                        OAM_item->y = sin_table[SPRITE_START_POSITION];
                        OAM_item->x = (uint8_t)(SPRITE_START_POSITION);
                        OAM_item->prop = 0;
                    }
                    break;
                }
            }
        }

        // move scroll
        if (display_head != display_tail) {
            for (uint8_t i = display_tail; i != display_head; ) {
                OAM_item_t * OAM_item = shadow_OAM + display[i = ((i + 1) & QUEUE_MASK)];
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
            if (SCY_REG < scroll_limit) SCY_REG++;
        }

        vsync();
    }
}

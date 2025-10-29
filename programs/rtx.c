#include <go-board.h>
#include <ssd1331.h>
#include <random.h>
#include <stdint.h>
#include <vec3.h>
#include <fxp.h>

typedef struct _sphere {
    _vec3 pos;
    fxp32_t radius;
} _sphere;

typedef struct _ray {
    _vec3 origin, dir;
} _ray;


_fast int main(void) {
    ssd1331_init();

    ssd1331_set_addr_window(0, 0, SSD1331_WIDTH, SSD1331_HEIGHT);
    ssd1331_cmd0(SSD1331_CMD_WRITE_RAM);
    ssd1331_stream_begin();

    for (uint8_t y = 0; y < SSD1331_HEIGHT; y++) {
        for (uint8_t x = 0; x < SSD1331_WIDTH; x++) {
            fxp32_t dir_x = fxp_mul(((int32_to_fxp(x) / (SSD1331_WIDTH - 1)) * 2 - FXP_ONE), SSD1331_ASPECT_RATIO);
            fxp32_t dir_y = (int32_to_fxp(y) / (SSD1331_HEIGHT - 1)) * 2 - FXP_ONE;
            fxp32_t dir_z = FXP_ONE;

            _ray ray = (_ray){
                .origin = {0, 0, 0},
                .dir = {dir_x, dir_y, dir_z}
            };

            ssd1331_send_vec3(normalize(ray.dir));
        }
    }

    ssd1331_stream_end();

    return 0;
}

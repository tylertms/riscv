#include <go-board.h>
#include <ssd1331.h>
#include <random.h>
#include <stdint.h>
#include <vec3.h>
#include <fxp.h>

#define NUM_SPHERES 1

typedef struct _sphere {
    _vec3 pos;
    fxp32_t radius;
} _sphere;

typedef struct _ray {
    _vec3 origin, dir;
} _ray;

_fast fxp32_t hit_sphere(_sphere* sphere, _ray* ray) {
    _vec3 offset = vec3_sub_vec3(sphere->pos, ray->origin);
    fxp32_t a = vec3_lensqr(ray->dir);
    fxp32_t h = vec3_dot(ray->dir, offset);
    fxp32_t c = vec3_lensqr(offset) - fxp_mul(sphere->radius, sphere->radius);
    fxp32_t discriminant = fxp_mul(h, h) - fxp_mul(a, c);

    if (discriminant < 0) {
        return -FXP_ONE;
    } else {
        return fxp_div(h - fxp_sqrt(discriminant), a);
    }
}

_fast int main(void) {
    _sphere spheres[NUM_SPHERES] = {
        {
            .pos = {0, 0, float_to_fxp(2.f)},
            .radius = float_to_fxp(1.f)
        }
    };

    ssd1331_init();

    ssd1331_set_addr_window(0, 0, SSD1331_WIDTH, SSD1331_HEIGHT);
    ssd1331_cmd0(SSD1331_CMD_WRITE_RAM);
    ssd1331_stream_begin();

    fxp32_t dir_z = float_to_fxp(1.f);

    for (uint8_t y = 0; y < SSD1331_HEIGHT; y++) {
        for (uint8_t x = 0; x < SSD1331_WIDTH; x++) {
            fxp32_t dir_x = fxp_mul(((int32_to_fxp(x) / (SSD1331_WIDTH - 1)) * 2 - FXP_ONE), SSD1331_ASPECT_RATIO);
            fxp32_t dir_y = (int32_to_fxp(y) / (SSD1331_HEIGHT - 1)) * 2 - FXP_ONE;

            _ray ray = (_ray){
                .origin = {0, 0, 0},
                .dir = {dir_x, dir_y, dir_z}
            };

            ssd1331_send_vec3(hit_sphere(&spheres[0], &ray) >= 0 ? ray.dir : (_vec3){0, 0, 0});
        }
    }

    ssd1331_stream_end();

    return 0;
}

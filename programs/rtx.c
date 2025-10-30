#include <go-board.h>
#include <ssd1331.h>
#include <random.h>
#include <stdint.h>
#include <vec3.h>
#include <fxp.h>
#include <string.h>

#define NUM_SPHERES 2
#define NUM_PLANES 5
#define DIST_MIN (fxp32_t)(FXP_ONE / 10000)
#define RAYS_PER_PIXEL 4
#define MAX_BOUNCES 1024

typedef struct _ray {
    _vec3 origin, dir;
} _ray;

typedef struct _sphere {
    _vec3 pos;
    fxp32_t radius;
    _vec3 color;
    uint8_t emit;
} _sphere;

typedef struct _plane {
    _vec3 point, normal;
    _vec3 color;
    uint8_t emit;
} _plane;

typedef struct _hit {
    _vec3 pos, normal;
    fxp32_t dist;
    uint8_t did_hit;
    _vec3 color;
    uint8_t emit;
} _hit;

_fast _hit hit_sphere(const _sphere* sphere, _ray* ray) {
    _vec3 offset = vec3_sub_vec3(sphere->pos, ray->origin);
    fxp32_t a = vec3_lensqr(ray->dir);
    fxp32_t h = vec3_dot(ray->dir, offset);
    fxp32_t c = vec3_lensqr(offset) - fxp_mul(sphere->radius, sphere->radius);
    fxp32_t discriminant = fxp_mul(h, h) - fxp_mul(a, c);

    _hit hit;
    hit.did_hit = 0;

    if (discriminant <= 0)
        return hit;

    fxp32_t sqrtd = fxp_sqrt(discriminant);
    fxp32_t root = fxp_div(h - sqrtd, a);

    if (root <= DIST_MIN) {
        root = fxp_div(h + sqrtd, a);
        if (root <= DIST_MIN)
            return hit;
    }

    hit.dist = root;
    hit.pos = vec3_add_vec3(ray->origin, vec3_mul_fxp(ray->dir, hit.dist));
    hit.normal = vec3_div_fxp(vec3_sub_vec3(hit.pos, sphere->pos), sphere->radius);
    hit.did_hit = 1;

    return hit;
}

_fast _hit hit_plane(const _plane* plane, _ray* ray) {
    fxp32_t denom = vec3_dot(plane->normal, ray->dir);
    _hit hit;
    hit.did_hit = 0;

    if (fxp_abs(denom) <= 1)
        return hit;

    _vec3 offset = vec3_sub_vec3(plane->point, ray->origin);
    fxp32_t root = fxp_div(vec3_dot(offset, plane->normal), denom);

    if (root <= DIST_MIN)
        return hit;

    hit.dist = root;
    hit.pos = vec3_add_vec3(ray->origin, vec3_mul_fxp(ray->dir, hit.dist));
    hit.normal = plane->normal;

    if (denom > 0)
        hit.normal = vec3_neg(hit.normal);

    hit.did_hit = 1;
    return hit;
}

_fast static const _hit ray_hit(_ray* ray) {
    static const _sphere spheres[NUM_SPHERES] = {
        {
            .pos = {0, 2 * FXP_ONE, (fxp32_t)(16.f * FXP_ONE)},
            .radius = 2 * FXP_ONE,
            .color = {FXP_ONE, FXP_ONE, FXP_ONE},
            .emit = 0
        },
        {
            .pos = {0, (fxp32_t)(-11.5f * FXP_ONE), 15 * FXP_ONE},
            .radius = 8 * FXP_ONE,
            .color = {FXP_ONE, FXP_ONE, FXP_ONE},
            .emit = 3
        }
    };

    static const _plane planes[NUM_PLANES] = {
        {
            .point = {-4 * FXP_ONE, 0, 0},
            .normal = {FXP_ONE, 0, 0},
            .color = {FXP_ONE, 0, 0}
        },
        {
            .point = {4 * FXP_ONE, 0, 0},
            .normal = {-FXP_ONE, 0, 0},
            .color = {0, FXP_ONE, 0}
        },
        {
            .point = {0, -4 * FXP_ONE, 0},
            .normal = {0, FXP_ONE, 0},
            .color = {FXP_ONE, FXP_ONE, FXP_ONE},
        },
        {
            .point = {0, 4 * FXP_ONE, 0},
            .normal = {0, -FXP_ONE, 0},
            .color = {FXP_ONE, FXP_ONE, FXP_ONE}
        },
        {
            .point = {0, 0, 20 * FXP_ONE},
            .normal = {0, 0, -FXP_ONE},
            .color = {0, 0, FXP_ONE}
        }
    };

    _hit closest_hit;
    closest_hit.did_hit = 0;
    closest_hit.dist = INT32_MAX;

    for (int i = 0; i < NUM_SPHERES; i++) {
        _hit hit = hit_sphere(&spheres[i], ray);
        if (hit.did_hit && (hit.dist < closest_hit.dist)) {
            closest_hit = hit;
            closest_hit.color = spheres[i].color;
            closest_hit.emit = spheres[i].emit;
        }
    }

    for (int i = 0; i < NUM_PLANES; i++) {
        _hit hit = hit_plane(&planes[i], ray);
        if (hit.did_hit && (hit.dist < closest_hit.dist)) {
            closest_hit = hit;
            closest_hit.color = planes[i].color;
            closest_hit.emit = planes[i].emit;
        }
    }

    return closest_hit;
}

_vec3 get_color(_ray* ray) {
    _vec3 incoming_light = {0, 0, 0};
    _vec3 ray_color = {FXP_ONE, FXP_ONE, FXP_ONE};

    for (int i = 0; i < MAX_BOUNCES; i++) {
        _hit hit = ray_hit(ray);
        if (!hit.did_hit) {
            break;
        };

        ray->origin = vec3_add_vec3(hit.pos, vec3_mul_fxp(hit.normal, DIST_MIN));
        ray->dir = vec3_normalize(vec3_add_vec3(hit.normal, rand_dir()));

        _vec3 emitted_light = vec3_mul_int32(hit.color, hit.emit);
        incoming_light = vec3_add_vec3(incoming_light, vec3_mul_vec3(emitted_light, ray_color));
        ray_color = vec3_mul_vec3(ray_color, hit.color);
    }

    return incoming_light;
}

int main(void) {
    uint32_t rendered_pixels = 0;

    ssd1331_init();

    ssd1331_set_addr_window(16, 0, SSD1331_HEIGHT, SSD1331_HEIGHT);
    ssd1331_cmd0(SSD1331_CMD_WRITE_RAM);
    ssd1331_stream_begin();

    for (uint8_t y = 0; y < SSD1331_HEIGHT; y++) {
        for (uint8_t x = 0; x < SSD1331_HEIGHT; x++) {
            _vec3 color = {0, 0, 0};

            for (uint16_t s = 0; s < RAYS_PER_PIXEL; s++) {
                fxp32_t pos_x = int32_to_fxp(x) + (random32() & FRAC_MASK);
                fxp32_t pos_y = int32_to_fxp(y) + (random32() & FRAC_MASK);
                fxp32_t dir_x = pos_x / (SSD1331_HEIGHT - 1) * 2 - FXP_ONE;
                fxp32_t dir_y = pos_y / (SSD1331_HEIGHT - 1) * 2 - FXP_ONE;

                _ray ray = (_ray){
                    .origin = {0, 0, 0},
                    .dir = vec3_normalize((_vec3){dir_x / 6, dir_y / 6, FXP_ONE / 2})
                };

                color = vec3_add_vec3(color, get_color(&ray));
            }

            ssd1331_send_vec3(vec3_div_int32(color, RAYS_PER_PIXEL));

            rendered_pixels++;
            IO_OUT(IO_SEG_ONE, to_seg((rendered_pixels >> 4) % 0xF));
            IO_OUT(IO_SEG_TWO, to_seg(rendered_pixels & 0xF));
        }
    }

    ssd1331_stream_end();

    return 0;
}

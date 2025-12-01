#include "ssd1331.h"
#include "fxp.h"

// Parameters
#define CUBE_SZ   (fxp32_t)((1 << 6) << FRAC_BITS)
#define CAM_DIST  (fxp32_t)((1 << 8) << FRAC_BITS)
#define CTR_X     (fxp32_t)(48 << FRAC_BITS)
#define CTR_Y     (fxp32_t)(32 << FRAC_BITS)

// Sine LUT
_rodata const fxp32_t sin_quarter[65] = {
         0,   1608,   3216,   4821,   6424,   8022,   9616,  11204,
     12785,  14359,  15924,  17479,  19024,  20557,  22078,  23586,
     25080,  26558,  28020,  29466,  30893,  32303,  33692,  35062,
     36410,  37736,  39040,  40320,  41576,  42806,  44011,  45190,
     46341,  47464,  48559,  49624,  50660,  51665,  52639,  53581,
     54491,  55368,  56212,  57022,  57798,  58538,  59244,  59914,
     60547,  61145,  61705,  62228,  62714,  63162,  63572,  63944,
     64277,  64571,  64827,  65043,  65220,  65358,  65457,  65516,
     65536
};

// Cube vertices
_rodata const int8_t verts[24] = {
    -1,-1,-1,  1,-1,-1,  1, 1,-1, -1, 1,-1,
    -1,-1, 1,  1,-1, 1,  1, 1, 1, -1, 1, 1
};

// Point-index connections
_rodata const uint8_t edges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0}, {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7}
};

// Edge colors
_rodata const uint8_t colors[12][3] = {
    {255,0,0}, {255,127,0}, {255,255,0}, {127,255,0},
    {0,255,0}, {0,255,127}, {0,255,255}, {0,127,255},
    {0,0,255}, {127,0,255}, {255,0,255}, {255,0,127}
};

// Use Sine LUT
_text fxp32_t get_sin_val(uint8_t i) {
    uint8_t phase = i & 0x7F;
    if (phase > 64) phase = 128 - phase;
    if (phase > 64) phase = 64;

    fxp32_t val = sin_quarter[phase];
    return (i & 0x80) ? -val : val;
}

// Convert to use Sine LUT
_text fxp32_t get_cos_val(uint8_t i) {
    return get_sin_val((uint8_t)(i + 64));
}

_text void transform_vertex(int v_idx, uint8_t ax, uint8_t ay, uint8_t az, int8_t* out_x, int8_t* out_y) {
    // Get trig values
    fxp32_t sx = get_sin_val(ax), cx = get_cos_val(ax);
    fxp32_t sy = get_sin_val(ay), cy = get_cos_val(ay);
    fxp32_t sz = get_sin_val(az), cz = get_cos_val(az);

    fxp32_t x = (verts[v_idx*3]   > 0) ? CUBE_SZ : -CUBE_SZ;
    fxp32_t y = (verts[v_idx*3+1] > 0) ? CUBE_SZ : -CUBE_SZ;
    fxp32_t z = (verts[v_idx*3+2] > 0) ? CUBE_SZ : -CUBE_SZ;

    fxp32_t tmp;

    // Rotate in all directions
    tmp = fxp_mul(y, cx) - fxp_mul(z, sx);
    z = fxp_mul(y, sx) + fxp_mul(z, cx);
    y = tmp;

    tmp = fxp_mul(x, cy) - fxp_mul(z, sy);
    z = fxp_mul(x, sy) + fxp_mul(z, cy);
    x = tmp;

    tmp = fxp_mul(x, cz) - fxp_mul(y, sz);
    y = fxp_mul(x, sz) + fxp_mul(y, cz);
    x = tmp;

    fxp32_t z_depth = z + CAM_DIST;
    if (z_depth < FXP_ONE) z_depth = FXP_ONE;

    // Get projection
    fxp32_t ratio_x = fxp_div(x, z_depth);
    fxp32_t ratio_y = fxp_div(y, z_depth);

    // Map to screen (X,Y)
    fxp32_t px = fxp_to_int32(fxp_mul(ratio_x, CTR_X) + CTR_X);
    fxp32_t py = fxp_to_int32(fxp_mul(ratio_y, CTR_X) + CTR_Y);

    // Clamp to screen
    if (px < 0) {
        px = 0;
    } else if (px >= SSD1331_WIDTH) {
        px = SSD1331_WIDTH - 1;
    }

    if (py < 0) {
        py = 0;
    } else if (py >= SSD1331_HEIGHT) {
        py = SSD1331_HEIGHT - 1;
    }

    *out_x = (int8_t)px;
    *out_y = (int8_t)py;
}

_text void draw_wireframe(int8_t cur[8][2], int8_t old[8][2]) {
    // For each edge
    for (int i = 0; i < 12; i++) {
        // Get the points for that edge
        uint8_t p1 = edges[i][0];
        uint8_t p2 = edges[i][1];

        if (cur[p1][0] != old[p1][0] || cur[p1][1] != old[p1][1] ||
            cur[p2][0] != old[p2][0] || cur[p2][1] != old[p2][1]) {
            draw_line_color(old[p1][0], old[p1][1], old[p2][0], old[p2][1], 0, 0, 0);
        }

        // Draw between the two (X,Y) using the SSD1331 graphics accel command
        // With that edge's corresponding color
        draw_line_color(cur[p1][0], cur[p1][1], cur[p2][0], cur[p2][1],
                        colors[i][0], colors[i][1], colors[i][2]);
    }
}

/* ---------------- Main ---------------- */
_text int main(void) {
    ssd1331_init();
    ssd1331_cmd4(SSD1331_CLEAR_WINDOW, 0, 0, SSD1331_WIDTH-1, SSD1331_HEIGHT-1);
    delay_ms(10);

    int8_t cur[8][2];
    int8_t old[8][2];

    for(int i=0; i<8; i++) {
        old[i][0] = SSD1331_WIDTH / 2;
        old[i][1] = SSD1331_HEIGHT / 2;
    }

    uint8_t ax = 0, ay = 0, az = 0;

    while (1) {
        for (int i = 0; i < 8; i++) {
            transform_vertex(i, ax, ay, az, &cur[i][0], &cur[i][1]);
        }

        draw_wireframe(cur, old);

        for(int i = 0; i < 8; i++) {
            old[i][0] = cur[i][0];
            old[i][1] = cur[i][1];
        }

        ax += 1; ay += 2; az += 1;
        delay_ms(15);
    }
}

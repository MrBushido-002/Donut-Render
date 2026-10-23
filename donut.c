#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define WIDTH 160
#define HEIGHT 44
#define TWO_PI 6.2831853f

//define a 3D vector struct
typedef struct {float x, y, z;} Vec3;

// define a helper function to rotate a point around the x-axis
Vec3 rotateX(Vec3 p, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Vec3 r = { p.x, p.y * c - p.z * s, p.y * s + p.z * c };
    return r;
}


// define a helper function to rotate a point around the z-axis
Vec3 rotateZ(Vec3 p, float angle){
    float c = cosf(angle);
    float s = sinf(angle);
    Vec3 r = { p.x * c - p.y * s, p.x * s + p.y * c, p.z };
    return r;
}

int main(void){
    //---Donut and Camera Settings ---
    const float TUBE_RADIUS   = 1.0f;   // thickness of the donut's tube
    const float RING_RADIUS   = 2.0f;   // distance from donut center to tube center
    const float CAMERA_DIST   = 7.0f;   // how far the donut is from the viewer
    const float SCALE         = 22.0f;  // zoom (bigger = bigger donut on screen)

    // Characters from darkest to brightest
    const char shades[] = ".,-~:;=!*#$@";
    const int  num_shades = 12;

    // Direction pointing toward the light (up and toward the viewer)
    const Vec3 light = { 0.0f, 0.7071f, -0.7071f };

    float angleA = 0.0f;   // current tumble angle (around X)
    float angleB = 0.0f;   // current spin angle   (around Z)
 
    char  screen[WIDTH * HEIGHT];   // the characters to print
    float depth[WIDTH * HEIGHT];    // closeness of what's drawn in each cell
 
    printf("\x1b[2J");              // clear the terminal once


    for (;;) {
        //---clear the frame---
        memset(screen, ' ', sizeof(screen));
        for (int i = 0; i < WIDTH*HEIGHT; i++) depth[i] = 0.0f;


        
        // ---- 2. Walk over every point on the donut's surface ----
        // theta: angle around the tube (the small circle)
        // phi:   angle around the whole donut (the big circle)
        for (float theta = 0; theta < TWO_PI; theta += 0.07f) {
            for (float phi = 0; phi < TWO_PI; phi += 0.02f) {

                // (a) A point on the small circle, in the XY plane. Its outward direction is also the surface normal.
                Vec3 normal = {cosf(theta), sinf(theta), 0.0f};
                Vec3 point  = {RING_RADIUS + TUBE_RADIUS * normal.x, TUBE_RADIUS * normal.y, 0.0f};

                // (b) Sweep that circle around the Y axis by phi to turn the circle into a donut.
                float cp = cosf(phi);
                float sp = sinf(phi);
                Vec3 p = {  point.x * cp,  point.y, -point.x * sp };
                Vec3 n = { normal.x * cp, normal.y, -normal.x * sp };

                // (c) Rotate the whole donut by the animation angles.
                p = rotateZ(rotateX(p, angleA), angleB);
                n = rotateZ(rotateX(n, angleA), angleB);

                // (d) Push the donut away from the camera.
                p.z += CAMERA_DIST;

                // (e) Perspective projection: far things get smaller.
                float one_over_z = 1.0f / p.z;
                int col = (int)(WIDTH  / 2.0f + SCALE * one_over_z * p.x * 2.0f);
                int row = (int)(HEIGHT / 2.0f - SCALE * one_over_z * p.y);
                //  * 2.0 because terminal characters are about twice as tall as wide
                
                // (f) Brightness = how much the surface faces the light.
                float brightness = n.x * light.x + n.y * light.y + n.z * light.z;

                // (g) Draw it, but only if it's on screen and nearer
                //     than whatever was already drawn there (z-buffer).
                if (col >= 0 && col < WIDTH && row >= 0 && row < HEIGHT) {
                    int cell = col + WIDTH * row;
                    if (one_over_z > depth[cell]) {
                        depth[cell] = one_over_z;
                        int shade = brightness > 0 ? (int)(brightness * (num_shades - 1)) : 0;
                        screen[cell] = shades[shade];
                    }
                }
            }
        }
        // ---- 3. Print the frame ----
        printf("\x1b[H");   // move cursor to top-left (overwrite previous frame)
        for (int row = 0; row < HEIGHT; row++) {
            fwrite(&screen[row * WIDTH], 1, WIDTH, stdout);
            putchar('\n');
        }
        fflush(stdout);
 
        // ---- 4. Advance the animation ----
        angleA += 0.06f;
        angleB += 0.02f;
        usleep(30000);   // ~30 ms between frames
    }
    return 0;
}




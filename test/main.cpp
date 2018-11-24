﻿#include<dsp/fir.hpp>
#include<pffft.h>
#include<vector>
#include<iostream>
#include<../wdl/wavwrite.h>

#include <stdio.h>
#include <string.h>
#include <float.h>
#include <GLFW/glfw3.h>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

NSVGimage* g_image = NULL;

static unsigned char bgColor[4] = { 205,202,200,255 };
static unsigned char lineColor[4] = { 0,160,192,255 };

static float distPtSeg(float x, float y, float px, float py, float qx, float qy)
{
    float pqx, pqy, dx, dy, d, t;
    pqx = qx - px;
    pqy = qy - py;
    dx = x - px;
    dy = y - py;
    d = pqx*pqx + pqy*pqy;
    t = pqx*dx + pqy*dy;
    if (d > 0) t /= d;
    if (t < 0) t = 0;
    else if (t > 1) t = 1;
    dx = px + t*pqx - x;
    dy = py + t*pqy - y;
    return dx*dx + dy*dy;
}

static void cubicBez(float x1, float y1, float x2, float y2,
    float x3, float y3, float x4, float y4,
    float tol, int level)
{
    float x12, y12, x23, y23, x34, y34, x123, y123, x234, y234, x1234, y1234;
    float d;

    if (level > 12) return;

    x12 = (x1 + x2)*0.5f;
    y12 = (y1 + y2)*0.5f;
    x23 = (x2 + x3)*0.5f;
    y23 = (y2 + y3)*0.5f;
    x34 = (x3 + x4)*0.5f;
    y34 = (y3 + y4)*0.5f;
    x123 = (x12 + x23)*0.5f;
    y123 = (y12 + y23)*0.5f;
    x234 = (x23 + x34)*0.5f;
    y234 = (y23 + y34)*0.5f;
    x1234 = (x123 + x234)*0.5f;
    y1234 = (y123 + y234)*0.5f;

    d = distPtSeg(x1234, y1234, x1, y1, x4, y4);
    if (d > tol*tol) {
        cubicBez(x1, y1, x12, y12, x123, y123, x1234, y1234, tol, level + 1);
        cubicBez(x1234, y1234, x234, y234, x34, y34, x4, y4, tol, level + 1);
    }
    else {
        glVertex2f(x4, y4);
    }
}

void drawPath(float* pts, int npts, char closed, float tol)
{
    int i;
    glBegin(GL_LINE_STRIP);
    glColor4ubv(lineColor);
    glVertex2f(pts[0], pts[1]);
    for (i = 0; i < npts - 1; i += 3) {
        float* p = &pts[i * 2];
        cubicBez(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], tol, 0);
    }
    if (closed) {
        glVertex2f(pts[0], pts[1]);
    }
    glEnd();
}

void drawControlPts(float* pts, int npts)
{
    int i;

    // Control lines
    glColor4ubv(lineColor);
    glBegin(GL_LINES);
    for (i = 0; i < npts - 1; i += 3) {
        float* p = &pts[i * 2];
        glVertex2f(p[0], p[1]);
        glVertex2f(p[2], p[3]);
        glVertex2f(p[4], p[5]);
        glVertex2f(p[6], p[7]);
    }
    glEnd();

    // Points
    glPointSize(6.0f);
    glColor4ubv(lineColor);

    glBegin(GL_POINTS);
    glVertex2f(pts[0], pts[1]);
    for (i = 0; i < npts - 1; i += 3) {
        float* p = &pts[i * 2];
        glVertex2f(p[6], p[7]);
    }
    glEnd();

    // Points
    glPointSize(3.0f);

    glBegin(GL_POINTS);
    glColor4ubv(bgColor);
    glVertex2f(pts[0], pts[1]);
    for (i = 0; i < npts - 1; i += 3) {
        float* p = &pts[i * 2];
        glColor4ubv(lineColor);
        glVertex2f(p[2], p[3]);
        glVertex2f(p[4], p[5]);
        glColor4ubv(bgColor);
        glVertex2f(p[6], p[7]);
    }
    glEnd();
}

void drawframe(GLFWwindow* window)
{
    int width = 0, height = 0;
    float view[4], cx, cy, hw, hh, aspect, px;
    NSVGshape* shape;
    NSVGpath* path;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);
    glClearColor(220.0f / 255.0f, 220.0f / 255.0f, 220.0f / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Fit view to bounds
    cx = g_image->width*0.5f;
    cy = g_image->height*0.5f;
    hw = g_image->width*0.5f;
    hh = g_image->height*0.5f;

    if (width / hw < height / hh) {
        aspect = (float)height / (float)width;
        view[0] = cx - hw * 1.2f;
        view[2] = cx + hw * 1.2f;
        view[1] = cy - hw * 1.2f * aspect;
        view[3] = cy + hw * 1.2f * aspect;
    }
    else {
        aspect = (float)width / (float)height;
        view[0] = cx - hh * 1.2f * aspect;
        view[2] = cx + hh * 1.2f * aspect;
        view[1] = cy - hh * 1.2f;
        view[3] = cy + hh * 1.2f;
    }
    // Size of one pixel.
    px = (view[2] - view[1]) / (float)width;

    glOrtho(view[0], view[2], view[3], view[1], -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glColor4ub(255, 255, 255, 255);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw bounds
    glColor4ub(0, 0, 0, 64);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0, 0);
    glVertex2f(g_image->width, 0);
    glVertex2f(g_image->width, g_image->height);
    glVertex2f(0, g_image->height);
    glEnd();

    for (shape = g_image->shapes; shape != NULL; shape = shape->next) {
        for (path = shape->paths; path != NULL; path = path->next) {
            drawPath(path->pts, path->npts, path->closed, px * 1.5f);
            drawControlPts(path->pts, path->npts);
        }
    }

    glfwSwapBuffers(window);
}

void resizecb(GLFWwindow* window, int width, int height)
{
    // Update and render
    NSVG_NOTUSED(width);
    NSVG_NOTUSED(height);
    drawframe(window);
}



#define Box_Size 8192
int main()
{

    using namespace std;
    vector<float>::iterator it;
    vector<float> impulse(Box_Size, 0.0f);
    //impulse[0] = 1.0f;
    rack::blackmanHarrisWindow(impulse.data(), Box_Size);
    /*for(it=impulse.begin();it!=impulse.end();++it)
    {
    
        cout << *it << endl;
    }*/
    float *in, *out;


    in = (float*)pffft_aligned_malloc(Box_Size*sizeof(float));
    for (int i=0;i<impulse.size();++i)
    {
        in[i] = impulse[i];
    }
    out = (float*)pffft_aligned_malloc(Box_Size * sizeof(float));
    memset(out, 0, Box_Size * sizeof(float));
    PFFFT_Setup* setup = pffft_new_setup(Box_Size, pffft_transform_t::PFFFT_REAL);
    pffft_transform_ordered(setup, in, out, nullptr, PFFFT_FORWARD);
    for (int i = 0; i < Box_Size; ++i)
    {
        out[i] = out[i] / Box_Size;
    }
    WaveWriter writer("E:/boxcar.wav", 16, 1, 44100,0);
    writer.WriteFloats(impulse.data(),Box_Size);
    
    for (int i = 0; i < Box_Size; i += 2)
    {
        
        in[i / 2] = std::pow((std::pow(out[i], 2) + std::pow(out[i + 1], 2)), 0.5);

    }
    WaveWriter writer2("E:/boxcar1.wav", 16, 1, 44100,0);
    writer2.WriteFloats(in,Box_Size/2);

    pffft_transform_ordered(setup, out, in,nullptr,PFFFT_BACKWARD);
    WaveWriter writer3("E:/boxcar2.wav", 16, 1, 44100,0);
    writer3.WriteFloats(in, Box_Size);
    
    pffft_aligned_free(in);
    pffft_aligned_free(out);
    pffft_destroy_setup(setup);
    
    GLFWwindow* window;
    const GLFWvidmode* mode;

    if (!glfwInit())
        return -1;
    
    mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    window = glfwCreateWindow(mode->width - 40, mode->height - 80, "Nano SVG", NULL, NULL);
    if (!window)
    {
        printf("Could not open window\n");
        glfwTerminate();
        return -1;
    }
    
    glfwSetFramebufferSizeCallback(window, resizecb);
    glfwMakeContextCurrent(window);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    
    
    g_image = nsvgParseFromFile("E:/Codes/Rack/dep/nanosvg/example/nano.svg", "px", 96.0f);
    if (g_image == NULL) {
        printf("Could not open SVG image.\n");
        glfwTerminate();
        return -1;
    }
    
    while (!glfwWindowShouldClose(window))
    {
        drawframe(window);
        glfwPollEvents();
    }
    
    nsvgDelete(g_image);

    glfwTerminate();

}
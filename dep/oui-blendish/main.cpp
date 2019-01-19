#define NANOVG_GLEW
#include <stdio.h>
#ifdef NANOVG_GLEW
#	include <GL/glew.h>
#endif
#ifdef __APPLE__
#	define GLFW_INCLUDE_GLCOREARB
#endif
#include <GLFW/glfw3.h>
#include "nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"

#define BLENDISH_IMPLEMENTATION
#include "blendish.h"

#define OUI_IMPLEMENTATION
#include "oui.h"



typedef struct {
	int subtype;
	UIhandler handler;
} UIData;

void ui_handler(int item, UIevent event) {
	UIData *data = (UIData *)uiGetHandle(item);
	if (data && data->handler) {
		data->handler(item, event);
	}
}

static struct NVGcontext *vg = NULL;

int main(int argc,const char *argv[])
{
	if (glfwInit())
	{
		printf("%s", "glfw init !");
	}
	GLFWwindow *window;
	UIcontext *contex;
	contex = uiCreateContext(4091, 1 << 20);
	uiMakeCurrent(contex);
	uiSetHandler(ui_handler);

	window=glfwCreateWindow(400, 500, "Blendish_Test", NULL, NULL);
	
	if (!window)
	{
		glfwTerminate();
		printf("window init fail");
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit()!=GLEW_OK)
	{
		printf("GLEW Init failed");

	}

	vg = nvgCreateGL3(NVG_ANTIALIAS);

	bndSetFont(nvgCreateFont(vg, "system", "C:/wkspace/L_Rack/dep/oui-blendish/DejaVuSans.ttf"));
	bndSetIconImage(nvgCreateImage(vg, "C:/wkspace/L_Rack/dep/oui-blendish/blender_icons16.png", 0));


	glfwSwapInterval(0);

	glfwSetTime(0);
	double c = 0.0;
	int total = 0;

	int peak_items = 0;
	unsigned int peak_alloc = 0;
	while (!glfwWindowShouldClose(window))
	{
		double mx, my;
		int winWidth, winHeight;
		int fbWidth, fbHeight;
		float pxRatio;

		glfwGetCursorPos(window, &mx, &my);
		glfwGetWindowSize(window, &winWidth, &winHeight);
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
		// Calculate pixel ration for hi-dpi devices.
		pxRatio = (float)fbWidth / (float)winWidth;

		// Update and render
		glViewport(0, 0, fbWidth, fbHeight);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		double t = glfwGetTime();
		nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

		bndToolButton(vg, 0, 0, 100, 100,	BNDcornerFlags::BND_CORNER_NONE, BNDwidgetState::BND_DEFAULT,
			BND_ICON_AUTO, "let's do it");

		nvgEndFrame(vg);
		double t2 = glfwGetTime();
		c += (t2 - t);
		total++;
		if (total > (1 * 60)) {
			printf("%fms\n", (c / (double)total)*1000.0);
			total = 0;
			c = 0.0;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	uiDestroyContext(contex);

	nvgDeleteGL3(vg);

	glfwTerminate();
	return 0;




}
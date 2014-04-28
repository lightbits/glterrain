#include <app/glcontext.h>
#include <common/timer.h>
#include <sstream>
#include <string>

static GLContext *g_active_context = nullptr;
GLContext *getActiveContext() {
	return g_active_context;
}

GLContext::GLContext() : window(nullptr), context(nullptr)
{ }

bool GLContext::create(const VideoMode &mode, const char *title, bool decorated, bool centered)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		return false;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, mode.GlMajor);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, mode.GlMinor);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, mode.DepthBits);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, mode.FsaaSamples > 0 ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, mode.FsaaSamples);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	int x = centered ? SDL_WINDOWPOS_CENTERED : SDL_WINDOWPOS_UNDEFINED;
	int y = centered ? SDL_WINDOWPOS_CENTERED : SDL_WINDOWPOS_UNDEFINED;

	window = SDL_CreateWindow(
		title, x, y, mode.Width, mode.Height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

	if (window == NULL)
		return false;

	context = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(mode.Vsync ? 1 : 0);

	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK)
	{
		SDL_Quit();
		return false;
	}

	g_active_context = this;

	return true;
}

void GLContext::close()
{
	g_active_context = nullptr;
}

void GLContext::dispose()
{
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	g_active_context = nullptr;
}

std::string GLContext::getDebugInfo() const
{
	std::stringstream ss;
	int context_flags;
	int accelerated;
	int double_buffer;
	int depth_bits;
	int stencil_bits;
	int fsaa_samples;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &context_flags);
	SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &accelerated);
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &double_buffer);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth_bits);
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stencil_bits);
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &fsaa_samples);
	ss << "Debug context: "	 << (context_flags & SDL_GL_CONTEXT_DEBUG_FLAG ? "yes" : "no") << std::endl;
	ss << "HW accelerated: " << (accelerated ? "yes" : "no") << std::endl;
	ss << "Doublebuffered: " << (double_buffer ? "yes" : "no") << std::endl;
	ss << "Depth bits: "     << depth_bits << std::endl;
	ss << "Stencil bits: "   << stencil_bits << std::endl;
	ss << "FSAA samples: "   << fsaa_samples << std::endl;
	ss << "Vendor: "         << glGetString(GL_VENDOR) << std::endl;
	ss << "Renderer: "       << glGetString(GL_RENDERER) << std::endl;
	ss << "GL ver.: "        << glGetString(GL_VERSION) << std::endl;
	ss << "GLSL ver.: "	     << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	return ss.str();
}

void GLContext::pollEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYUP:
			if (!key_released)
				break;
			key_released(event.key.keysym.mod, event.key.keysym.sym);
			break;
		case SDL_KEYDOWN:
			if (!key_pressed)
				break;
			key_pressed(event.key.keysym.mod, event.key.keysym.sym);
			break;
		case SDL_MOUSEMOTION:
			if (mouse_dragged && event.motion.state & SDL_BUTTON_LMASK)
				mouse_dragged(SDL_BUTTON_LEFT, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
			else if (mouse_dragged && event.motion.state & SDL_BUTTON_MMASK)
				mouse_dragged(SDL_BUTTON_MIDDLE, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
			else if (mouse_dragged && event.motion.state & SDL_BUTTON_RMASK)
				mouse_dragged(SDL_BUTTON_RIGHT, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
			else if (mouse_moved)
				mouse_moved(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (!mouse_pressed)
				break;
			mouse_pressed(event.button.button, event.button.x, event.button.y);
			break;
		case SDL_MOUSEBUTTONUP:
			if (!mouse_released)
				break;
			mouse_released(event.button.button, event.button.x, event.button.y);
			break;
		case SDL_QUIT:
			close();
			break;
		}
	}
}

void GLContext::display()
{
	SDL_GL_SwapWindow(window);
}

void GLContext::screenshot(const char *filename, int x, int y, int w, int h)
{
	w = w == 0 ? getWidth() : w;
	h = h == 0 ? getHeight() : h;
	unsigned char *pixels = new unsigned char[w * h * 3];
	glReadPixels(x, y, w, h, GL_BGR, GL_UNSIGNED_BYTE, pixels);

	// Flip pixel buffer
	unsigned char *flipped_pixels = new unsigned char[w * h * 3];
	int num_pixels = w * h;
	for (int i = 0; i < num_pixels; ++i)
	{
		int x = i % w;
		int y = i / w;

		int offset0 = (y * w + x) * 3;
		int offset1 = ((h - 1 - y) * w + x) * 3;

		flipped_pixels[offset0] = pixels[offset1];
		flipped_pixels[offset0 + 1] = pixels[offset1 + 1];
		flipped_pixels[offset0 + 2] = pixels[offset1 + 2];
	}

	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(flipped_pixels, w, h, 24, w * 3, 0, 0, 0, 0);
	SDL_SaveBMP(surface, filename);

	SDL_FreeSurface(surface);
	delete[] pixels;
	delete[] flipped_pixels;
}

void GLContext::setCursorEnabled(bool cursor)
{
	SDL_ShowCursor(cursor);
}

void GLContext::setWindowTitle(const char *title)
{
	SDL_SetWindowTitle(window, title);
}

void GLContext::setWindowPosition(int x, int y)
{
	SDL_SetWindowPosition(window, x, y);
}

void GLContext::setWindowSize(int w, int h)
{
	SDL_SetWindowSize(window, w, h);
}

void GLContext::setVerticalSync(bool vsync)
{
	if (!SDL_GL_SetSwapInterval(vsync))
		return; // log << could not enable vsync
}

void GLContext::getSize(int *width, int *height) const
{
	SDL_GetWindowSize(window, width, height);
}

int GLContext::getWidth() const
{
	int w, h; getSize(&w, &h);
	return w;
}

int GLContext::getHeight() const
{
	int w, h; getSize(&w, &h);
	return h;
}

void GLContext::getMousePos(int *x, int *y) const
{
	SDL_GetMouseState(x, y);
}

int GLContext::getMouseX() const
{ 
	int x, y;
	getMousePos(&x, &y);
	return x;
}

int GLContext::getMouseY() const
{
	int x, y;
	getMousePos(&x, &y);
	return y;
}		

void GLContext::setMousePos(int x, int y)
{
	SDL_WarpMouseInWindow(window, x, y);
}

double GLContext::getElapsedTime()
{
	return SDL_GetTicks() / 1000.0;
}

void GLContext::sleep(double seconds)
{
	SDL_Delay(Uint32(seconds * 1000));
}

void GLContext::sleepms(unsigned int milliseconds)
{
	SDL_Delay(milliseconds);
}

bool GLContext::isOpen() const
{
	return g_active_context == this;
}

bool GLContext::isKeyPressed(int scancode) const
{
	 const unsigned char *kbs = SDL_GetKeyboardState(NULL);
	 return kbs[scancode] != 0;
}

bool GLContext::isKeyPressed(char key) const
{
	 const unsigned char *kbs = SDL_GetKeyboardState(NULL);
	 int index = SDL_SCANCODE_A + int(tolower(key)) - int('a');
	 return kbs[index] != 0;
}

bool GLContext::isMousePressed(int button) const
{
	int state = SDL_GetMouseState(NULL, NULL);
	return SDL_BUTTON(state) == button;
}
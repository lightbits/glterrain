#include <common/input.h>

//void io::pollEvents(Context &ctx)
//{
//	SDL_Event event;
//	while (SDL_PollEvent(&event))
//	{
//		switch (event.type)
//		{
//		case SDL_KEYUP:
//			if (!key_released)
//				break;
//			Key key(event.key.keysym.sym
//			key_released(event.key.keysym.mod, event.key.keysym.sym);
//			break;
//		case SDL_KEYDOWN:
//			if (!key_pressed)
//				break;
//			key_pressed(event.key.keysym.mod, event.key.keysym.sym);
//			break;
//		case SDL_MOUSEMOTION:
//			if (mouse_dragged && event.motion.state & SDL_BUTTON_LMASK)
//				mouse_dragged(SDL_BUTTON_LEFT, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
//			else if (mouse_dragged && event.motion.state & SDL_BUTTON_MMASK)
//				mouse_dragged(SDL_BUTTON_MIDDLE, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
//			else if (mouse_dragged && event.motion.state & SDL_BUTTON_RMASK)
//				mouse_dragged(SDL_BUTTON_RIGHT, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
//			else if (mouse_moved)
//				mouse_moved(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
//			break;
//		case SDL_MOUSEBUTTONDOWN:
//			if (!mouse_pressed)
//				break;
//			mouse_pressed(event.button.button, event.button.x, event.button.y);
//			break;
//		case SDL_MOUSEBUTTONUP:
//			if (!mouse_released)
//				break;
//			mouse_released(event.button.button, event.button.x, event.button.y);
//			break;
//		case SDL_QUIT:
//			ctx.close();
//			break;
//		}
//	}
//}
#ifndef INPUT_H
#define INPUT_H
#include <functional>
#include <gl/opengl.h>
#include <app/context.h>

//namespace io
//{
//	struct Key
//	{
//		Key(char s, int c) : symbol(s), scancode(c) { }
//		char symbol;
//		int scancode;
//
//		bool operator==(char s) const { return symbol == s; } 
//		bool operator==(int c) const { return scancode == c; }
//	};
//
//	enum MouseButton { 
//		MouseButtonLeft = SDL_BUTTON_LEFT, 
//		MouseButtonMiddle = SDL_BUTTON_MIDDLE,
//		MouseButtonRight = SDL_BUTTON_RIGHT
//	};
//
//	typedef std::function< void(Key modifiers, Key key) > KeyboardEventCallback;
//	typedef std::function< void(int x, int y, int dx, int dy) > MouseMotionEventCallback;
//	typedef std::function< void(MouseButton button, int x, int y, int dx, int dy) > MouseDragEventCallback;
//	typedef std::function< void(MouseButton button, int x, int y) > MouseButtonEventCallback;
//
//	KeyboardEventCallback key_released;
//	KeyboardEventCallback key_pressed;
//	MouseMotionEventCallback mouse_moved;
//	MouseDragEventCallback mouse_dragged;
//	MouseButtonEventCallback mouse_pressed;
//	MouseButtonEventCallback mouse_released;
//
//	//void registerCallback(KeyboardEventCallback func);
//	//void registerCallback(MouseMotionEventCallback func);
//	//void registerCallback(MouseDragEventCallback func);
//	//void registerCallback(MouseButtonEventCallback func);
//
//	void pollEvents(Context &ctx);
//};

#endif
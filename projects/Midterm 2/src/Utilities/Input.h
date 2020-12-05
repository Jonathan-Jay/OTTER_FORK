#pragma once
#include <glfw/glfw3.h>

//in numerical order https://www.glfw.org/docs/3.3/group__keys.html
enum class KEY
{
	//32
	SPACE = GLFW_KEY_SPACE,

	//39
	APOSTROPHE = GLFW_KEY_APOSTROPHE,

	//44
	COMMA = GLFW_KEY_COMMA,

	//47
	PERIOD = GLFW_KEY_PERIOD,
	FSLASH,// = GLFW_KEY_SLASH,
	ZERO,// = GLFW_KEY_0,
	ONE,
	TWO,
	THREE,
	FOUR,
	FIVE,
	SIX,
	SEVEN,
	EIGHT,
	NINE,

	SEMICOLON = GLFW_KEY_SEMICOLON,

	//65
	A = GLFW_KEY_A,
	B, C, D, E, F,
	G, H, I, J, K,
	L, M, N, O, P,
	Q, R, S, T, U,
	V, W, X, Y, Z,
	LBRACKET,// = GLFW_KEY_LEFT_BRACKET,
	BSLASH,// = GLFW_KEY_BACKSLASH,
	RBRACKET,// = GLFW_KEY_RIGHT_BRACKET,

	//256
	ESC = GLFW_KEY_ESCAPE,
	ENTER,// = GLFW_KEY_ENTER,
	TAB,// = GLFW_KEY_TAB,
	BACKSPACE,// = GLFW_KEY_BACKSPACE,

	//262
	RIGHT = GLFW_KEY_RIGHT,
	LEFT,// = GLFW_KEY_LEFT,
	DOWN,// = GLFW_KEY_DOWN,
	UP,// = GLFW_KEY_UP,

	//280
	CAPS = GLFW_KEY_CAPS_LOCK,

	//290
	F1 = GLFW_KEY_F1,
	F2, F3, F4, F5, F6, F7,
	F8, F9, F10, F11, F12,
	F13, F14, F15, F16, F17,
	F18, F19, F20, F21, F22,
	F23, F24, F25,

	//340
	LSHIFT = GLFW_KEY_LEFT_SHIFT,
	LCTRL,// = GLFW_KEY_LEFT_CONTROL,
	LALT,// = GLFW_KEY_LEFT_ALT,
	LSUPER,// = GLFW_KEY_LEFT_SUPER		//IDK WHAT THIS IS
	RSHIFT,// = GLFW_KEY_RIGHT_SHIFT,
	RCTRL,// = GLFW_KEY_RIGHT_CONTROL
	RALT,// = GLFW_KEY_RIGHT_ALT,
	RSUPER,// = GLFW_KEY_RIGHT_SUPER,	//SAME

};

class Input abstract
{
public:
	//get window
	static void Init(GLFWwindow* win) { window = win; }
	//release window
	static void Unload() { window = nullptr; }

	//returns whether or not the key is held
	static bool GetKey(KEY key) {
		return glfwGetKey(window, int(key));
	}
	//returns if the key was just pressed down (on press but not hold)
	static bool GetKeyDown(KEY key) {
		if (!pressed[check(int(key))])
			return glfwGetKey(window, int(key));
		return false;
	}
	//returns if the key was just released (on release but not lack of input (use !GetKey))
	static bool GetKeyUp(KEY key) {
		if (pressed[check(int(key))])
			return !glfwGetKey(window, int(key));
		return false;
	}

	//checks all keys (kinda ineffecient)
	static void Update() {
		for (short i(0); i < __amt; ++i) {
			/*//91 - 32 = 59 and 256 - 32 = 224
			if (i == 59)
				i = 224;

			if (pressed[i])			//+ 32 because i shrunk the array
				if (!glfwGetKey(window, i + 32))
					pressed[i] = false;*/

			pressed[i] = glfwGetKey(window, i + (i <= __mid2 ? __lowest : __last));
		}
	}

private:
	static int check(int test) { return test - (test <= __mid ? __lowest : __last); }
	static GLFWwindow* window;
	static const short __lowest = 32;				//lowest character index
	static const short __mid = 93;					//top of lower end
	static const short __mid2 = __mid - __lowest;
	static const short __last = 256 - __mid2 - 1;	//start of upper
	static const short __amt = 347 - __last;		//highest of upper
	static bool pressed[__amt];
};

inline GLFWwindow* Input::window = nullptr;
inline bool Input::pressed[__amt] = { false };


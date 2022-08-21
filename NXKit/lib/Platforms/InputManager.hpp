//
//  InputManager.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 18.07.2022.
//

#pragma once

#include <Core/Geometry/Geometry.hpp>
#include <Core/Utils/NotificationEvent.hpp>
#include <Core/UITouch/UITouch.hpp>

#include <vector>

namespace NXKit {

typedef enum
{
    BUTTON_LT = 0,
    BUTTON_LB,

    BUTTON_LSB,

    BUTTON_UP,
    BUTTON_RIGHT,
    BUTTON_DOWN,
    BUTTON_LEFT,

    BUTTON_BACK,
    BUTTON_GUIDE,
    BUTTON_START,

    BUTTON_RSB,

    BUTTON_Y,
    BUTTON_B,
    BUTTON_A,
    BUTTON_X,

    BUTTON_RB,
    BUTTON_RT,

    BUTTON_NAV_UP,
    BUTTON_NAV_RIGHT,
    BUTTON_NAV_DOWN,
    BUTTON_NAV_LEFT,

    _BUTTON_MAX,
} ControllerButton;

// Abstract axis enum - names correspond to a generic Xbox controller
typedef enum
{
    LEFT_X,
    LEFT_Y,

    // No Z axis, LT and RT are in the buttons enum for the sake of simplicity

    RIGHT_X, // also called 5th axis
    RIGHT_Y, // also called 4th axis

    _AXES_MAX,
} ControllerAxis;

/// HidKeyboardScancode
/// Uses the same key codes as GLFW
typedef enum {
    /* The unknown key */
    BRLS_KBD_KEY_UNKNOWN            = -1,

    /* Printable keys */
    BRLS_KBD_KEY_SPACE              = 32,
    BRLS_KBD_KEY_APOSTROPHE         = 39,  /* ' */
    BRLS_KBD_KEY_COMMA              = 44,  /* , */
    BRLS_KBD_KEY_MINUS              = 45,  /* - */
    BRLS_KBD_KEY_PERIOD             = 46,  /* . */
    BRLS_KBD_KEY_SLASH              = 47,  /* / */
    BRLS_KBD_KEY_0                  = 48,
    BRLS_KBD_KEY_1                  = 49,
    BRLS_KBD_KEY_2                  = 50,
    BRLS_KBD_KEY_3                  = 51,
    BRLS_KBD_KEY_4                  = 52,
    BRLS_KBD_KEY_5                  = 53,
    BRLS_KBD_KEY_6                  = 54,
    BRLS_KBD_KEY_7                  = 55,
    BRLS_KBD_KEY_8                  = 56,
    BRLS_KBD_KEY_9                  = 57,
    BRLS_KBD_KEY_SEMICOLON          = 59,  /* ; */
    BRLS_KBD_KEY_EQUAL              = 61,  /* = */
    BRLS_KBD_KEY_A                  = 65,
    BRLS_KBD_KEY_B                  = 66,
    BRLS_KBD_KEY_C                  = 67,
    BRLS_KBD_KEY_D                  = 68,
    BRLS_KBD_KEY_E                  = 69,
    BRLS_KBD_KEY_F                  = 70,
    BRLS_KBD_KEY_G                  = 71,
    BRLS_KBD_KEY_H                  = 72,
    BRLS_KBD_KEY_I                  = 73,
    BRLS_KBD_KEY_J                  = 74,
    BRLS_KBD_KEY_K                  = 75,
    BRLS_KBD_KEY_L                  = 76,
    BRLS_KBD_KEY_M                  = 77,
    BRLS_KBD_KEY_N                  = 78,
    BRLS_KBD_KEY_O                  = 79,
    BRLS_KBD_KEY_P                  = 80,
    BRLS_KBD_KEY_Q                  = 81,
    BRLS_KBD_KEY_R                  = 82,
    BRLS_KBD_KEY_S                  = 83,
    BRLS_KBD_KEY_T                  = 84,
    BRLS_KBD_KEY_U                  = 85,
    BRLS_KBD_KEY_V                  = 86,
    BRLS_KBD_KEY_W                  = 87,
    BRLS_KBD_KEY_X                  = 88,
    BRLS_KBD_KEY_Y                  = 89,
    BRLS_KBD_KEY_Z                  = 90,
    BRLS_KBD_KEY_LEFT_BRACKET       = 91,  /* [ */
    BRLS_KBD_KEY_BACKSLASH          = 92,  /* \ */
    BRLS_KBD_KEY_RIGHT_BRACKET      = 93,  /* ] */
    BRLS_KBD_KEY_GRAVE_ACCENT       = 96,  /* ` */
    BRLS_KBD_KEY_WORLD_1            = 161, /* non-US #1 */
    BRLS_KBD_KEY_WORLD_2            = 162, /* non-US #2 */

    /* Function keys */
    BRLS_KBD_KEY_ESCAPE             = 256,
    BRLS_KBD_KEY_ENTER              = 257,
    BRLS_KBD_KEY_TAB                = 258,
    BRLS_KBD_KEY_BACKSPACE          = 259,
    BRLS_KBD_KEY_INSERT             = 260,
    BRLS_KBD_KEY_DELETE             = 261,
    BRLS_KBD_KEY_RIGHT              = 262,
    BRLS_KBD_KEY_LEFT               = 263,
    BRLS_KBD_KEY_DOWN               = 264,
    BRLS_KBD_KEY_UP                 = 265,
    BRLS_KBD_KEY_PAGE_UP            = 266,
    BRLS_KBD_KEY_PAGE_DOWN          = 267,
    BRLS_KBD_KEY_HOME               = 268,
    BRLS_KBD_KEY_END                = 269,
    BRLS_KBD_KEY_CAPS_LOCK          = 280,
    BRLS_KBD_KEY_SCROLL_LOCK        = 281,
    BRLS_KBD_KEY_NUM_LOCK           = 282,
    BRLS_KBD_KEY_PRINT_SCREEN       = 283,
    BRLS_KBD_KEY_PAUSE              = 284,
    BRLS_KBD_KEY_F1                 = 290,
    BRLS_KBD_KEY_F2                 = 291,
    BRLS_KBD_KEY_F3                 = 292,
    BRLS_KBD_KEY_F4                 = 293,
    BRLS_KBD_KEY_F5                 = 294,
    BRLS_KBD_KEY_F6                 = 295,
    BRLS_KBD_KEY_F7                 = 296,
    BRLS_KBD_KEY_F8                 = 297,
    BRLS_KBD_KEY_F9                 = 298,
    BRLS_KBD_KEY_F10                = 299,
    BRLS_KBD_KEY_F11                = 300,
    BRLS_KBD_KEY_F12                = 301,
    BRLS_KBD_KEY_F13                = 302,
    BRLS_KBD_KEY_F14                = 303,
    BRLS_KBD_KEY_F15                = 304,
    BRLS_KBD_KEY_F16                = 305,
    BRLS_KBD_KEY_F17                = 306,
    BRLS_KBD_KEY_F18                = 307,
    BRLS_KBD_KEY_F19                = 308,
    BRLS_KBD_KEY_F20                = 309,
    BRLS_KBD_KEY_F21                = 310,
    BRLS_KBD_KEY_F22                = 311,
    BRLS_KBD_KEY_F23                = 312,
    BRLS_KBD_KEY_F24                = 313,
    BRLS_KBD_KEY_F25                = 314,
    BRLS_KBD_KEY_KP_0               = 320,
    BRLS_KBD_KEY_KP_1               = 321,
    BRLS_KBD_KEY_KP_2               = 322,
    BRLS_KBD_KEY_KP_3               = 323,
    BRLS_KBD_KEY_KP_4               = 324,
    BRLS_KBD_KEY_KP_5               = 325,
    BRLS_KBD_KEY_KP_6               = 326,
    BRLS_KBD_KEY_KP_7               = 327,
    BRLS_KBD_KEY_KP_8               = 328,
    BRLS_KBD_KEY_KP_9               = 329,
    BRLS_KBD_KEY_KP_DECIMAL         = 330,
    BRLS_KBD_KEY_KP_DIVIDE          = 331,
    BRLS_KBD_KEY_KP_MULTIPLY        = 332,
    BRLS_KBD_KEY_KP_SUBTRACT        = 333,
    BRLS_KBD_KEY_KP_ADD             = 334,
    BRLS_KBD_KEY_KP_ENTER           = 335,
    BRLS_KBD_KEY_KP_EQUAL           = 336,
    BRLS_KBD_KEY_LEFT_SHIFT         = 340,
    BRLS_KBD_KEY_LEFT_CONTROL       = 341,
    BRLS_KBD_KEY_LEFT_ALT           = 342,
    BRLS_KBD_KEY_LEFT_SUPER         = 343,
    BRLS_KBD_KEY_RIGHT_SHIFT        = 344,
    BRLS_KBD_KEY_RIGHT_CONTROL      = 345,
    BRLS_KBD_KEY_RIGHT_ALT          = 346,
    BRLS_KBD_KEY_RIGHT_SUPER        = 347,
    BRLS_KBD_KEY_MENU               = 348,

    _BRLS_KBD_KEY_LAST
} BrlsKeyboardScancode;

typedef enum {
    BRLS_MOUSE_LKB = 0,
    BRLS_MOUSE_RKB = 1,
    BRLS_MOUSE_MKB = 2,
    _BRLS_MOUSE_LAST
} BrlsMouseButton;

class InputManager {
public:
    static InputManager* shared() { return _shared; }
    static void initWith(InputManager* shared) { _shared = shared; }

    virtual Point getCoursorPosition() = 0;

    virtual bool getMouseButton(BrlsMouseButton button) = 0;
    virtual bool getMouseButtonUp(BrlsMouseButton button) = 0;
    virtual bool getMouseButtonDown(BrlsMouseButton button) = 0;

    virtual bool getKey(BrlsKeyboardScancode key) = 0;
    virtual bool getKeyUp(BrlsKeyboardScancode key) = 0;
    virtual bool getKeyDown(BrlsKeyboardScancode key) = 0;

    virtual short getGamepadsCount() = 0;

    virtual bool getButton(short controller, ControllerButton button) = 0;
    virtual bool getButtonUp(short controller, ControllerButton button) = 0;
    virtual bool getButtonDown(short controller, ControllerButton button) = 0;
    
    virtual float getAxis(short controller, ControllerAxis axis) = 0;

    virtual int touchCount() = 0;
    virtual UITouch* getTouch(int id) = 0;
    virtual std::vector<UITouch*> getTouches() = 0;

    inline NotificationEvent<> *getInputUpdated() {
        return &inputUpdated;
    }
    
protected:
    virtual void update() = 0;

private:
    friend class Application;
    inline static InputManager* _shared = nullptr;
    NotificationEvent<> inputUpdated;
};

}

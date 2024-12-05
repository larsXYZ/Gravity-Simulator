////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Marco Antognini (antognini.marco@gmail.com),
//                         Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/OSX/WindowImplCocoa.hpp>

#import <SFML/Window/OSX/SFKeyboardModifiersHelper.h>

#if defined(__APPLE__)
    #if defined(__clang__)
        #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    #elif defined(__GNUC__)
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    #endif
#endif


////////////////////////////////////////////////////////////
/// Here are define the mask value for the 'modifiers'
/// keys (cmd, ctrl, alt, shift)
///
////////////////////////////////////////////////////////////
#define NSRightShiftKeyMask         0x020004
#define NSLeftShiftKeyMask          0x020002
#define NSRightCommandKeyMask       0x100010
#define NSLeftCommandKeyMask        0x100008
#define NSRightAlternateKeyMask     0x080040
#define NSLeftAlternateKeyMask      0x080020
#define NSRightControlKeyMask       0x042000
#define NSLeftControlKeyMask        0x040001


////////////////////////////////////////////////////////////
// Local Data Structures
////////////////////////////////////////////////////////////

/// Modifiers states
struct ModifiersState
{
    BOOL rightShiftWasDown;
    BOOL leftShiftWasDown;
    BOOL rightCommandWasDown;
    BOOL leftCommandWasDown;
    BOOL rightAlternateWasDown;
    BOOL leftAlternateWasDown;
    BOOL leftControlWasDown;
    BOOL rightControlWasDown;
    BOOL capsLockWasOn;
};


////////////////////////////////////////////////////////////
// Global Variables
////////////////////////////////////////////////////////////

/// Share 'modifiers' state with all windows to correctly fire pressed/released events
static ModifiersState state;
static BOOL isStateInitialized = NO;


////////////////////////////////////////////////////////////
// Local & Private Functions
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// \brief Carefully observe if the key mask is on in the modifiers
///
////////////////////////////////////////////////////////////
BOOL isKeyMaskActive(NSUInteger modifiers, NSUInteger mask);


////////////////////////////////////////////////////////////
/// \brief Handle one modifier key mask
///
/// Update the key state and send events to the requester
///
////////////////////////////////////////////////////////////
void processOneModifier(NSUInteger modifiers, NSUInteger mask,
                        BOOL& wasDown, sf::Keyboard::Key key,
                        sf::Keyboard::Scancode code,
                        sf::priv::WindowImplCocoa& requester);


////////////////////////////////////////////////////////////
/// \brief Handle left & right modifier keys
///
/// Update the keys state and send events to the requester
///
////////////////////////////////////////////////////////////
void processLeftRightModifiers(NSUInteger modifiers,
                               NSUInteger leftMask, NSUInteger rightMask,
                               BOOL& leftWasDown, BOOL& rightWasDown,
                               sf::Keyboard::Key leftKey, sf::Keyboard::Key rightKey,
                               sf::Keyboard::Scancode leftCode, sf::Keyboard::Scancode rightCode,
                               sf::priv::WindowImplCocoa& requester);



////////////////////////////////////////////////////////////
// Implementations
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
void initialiseKeyboardHelper(void)
{
    if (isStateInitialized) return;

    NSUInteger modifiers = [[NSApp currentEvent] modifierFlags];

    // Load current keyboard state
    state.leftShiftWasDown        = isKeyMaskActive(modifiers, NSLeftShiftKeyMask);
    state.rightShiftWasDown       = isKeyMaskActive(modifiers, NSRightShiftKeyMask);
    state.leftCommandWasDown      = isKeyMaskActive(modifiers, NSLeftCommandKeyMask);
    state.rightCommandWasDown     = isKeyMaskActive(modifiers, NSRightCommandKeyMask);
    state.leftAlternateWasDown    = isKeyMaskActive(modifiers, NSLeftAlternateKeyMask);
    state.rightAlternateWasDown   = isKeyMaskActive(modifiers, NSRightAlternateKeyMask);
    state.leftControlWasDown      = isKeyMaskActive(modifiers, NSLeftControlKeyMask);
    state.rightControlWasDown     = isKeyMaskActive(modifiers, NSRightControlKeyMask);
    state.capsLockWasOn           = isKeyMaskActive(modifiers, NSEventModifierFlagCapsLock);

    isStateInitialized = YES;
}


////////////////////////////////////////////////////////
sf::Event::KeyEvent keyEventWithModifiers(NSUInteger modifiers, sf::Keyboard::Key key, sf::Keyboard::Scancode code)
{
    sf::Event::KeyEvent event;
    event.code     = key;
    event.scancode = code;
    event.alt      = modifiers & NSAlternateKeyMask;
    event.control  = modifiers & NSControlKeyMask;
    event.shift    = modifiers & NSShiftKeyMask;
    event.system   = modifiers & NSCommandKeyMask;

    return event;
}


////////////////////////////////////////////////////////
void handleModifiersChanged(NSUInteger modifiers, sf::priv::WindowImplCocoa& requester)
{
    // Handle shift
    processLeftRightModifiers(
        modifiers,
        NSLeftShiftKeyMask, NSRightShiftKeyMask,
        state.leftShiftWasDown, state.rightShiftWasDown,
        sf::Keyboard::LShift, sf::Keyboard::RShift,
        sf::Keyboard::Scan::LShift, sf::Keyboard::Scan::RShift,
        requester
    );

    // Handle command
    processLeftRightModifiers(
        modifiers,
        NSLeftCommandKeyMask, NSRightCommandKeyMask,
        state.leftCommandWasDown, state.rightCommandWasDown,
        sf::Keyboard::LSystem, sf::Keyboard::RSystem,
        sf::Keyboard::Scan::LSystem, sf::Keyboard::Scan::RSystem,
        requester
    );

    // Handle option (alt)
    processLeftRightModifiers(
        modifiers,
        NSLeftAlternateKeyMask, NSRightAlternateKeyMask,
        state.leftAlternateWasDown, state.rightAlternateWasDown,
        sf::Keyboard::LAlt, sf::Keyboard::RAlt,
        sf::Keyboard::Scan::LAlt, sf::Keyboard::Scan::RAlt,
        requester
    );

    // Handle control
    processLeftRightModifiers(
        modifiers,
        NSLeftControlKeyMask, NSRightControlKeyMask,
        state.leftControlWasDown, state.rightControlWasDown,
        sf::Keyboard::LControl, sf::Keyboard::RControl,
        sf::Keyboard::Scan::LControl, sf::Keyboard::Scan::RControl,
        requester
    );

    // Handle caps lock
    processOneModifier(
        modifiers,
        NSEventModifierFlagCapsLock,
        state.capsLockWasOn,
        sf::Keyboard::Unknown,
        sf::Keyboard::Scan::CapsLock,
        requester
    );
}


////////////////////////////////////////////////////////
BOOL isKeyMaskActive(NSUInteger modifiers, NSUInteger mask)
{
    // Here we need to make sure it's exactly the mask since some masks
    // share some bits such that the & operation would result in a non zero
    // value without corresponding to the processed key.
    return (modifiers & mask) == mask;
}


////////////////////////////////////////////////////////
void processOneModifier(NSUInteger modifiers, NSUInteger mask,
                        BOOL& wasDown, sf::Keyboard::Key key,
                        sf::Keyboard::Scancode code,
                        sf::priv::WindowImplCocoa& requester)
{
    // Setup a potential event key.
    sf::Event::KeyEvent event = keyEventWithModifiers(modifiers, key, code);

    // State
    BOOL isDown = isKeyMaskActive(modifiers, mask);

    // Check for key pressed event
    if (isDown && !wasDown)
        requester.keyDown(event);

    // And check for key released event
    else if (!isDown && wasDown)
        requester.keyUp(event);

    // else isDown == wasDown, so no change

    // Update state
    wasDown = isDown;
}


////////////////////////////////////////////////////////
void processLeftRightModifiers(NSUInteger modifiers,
                               NSUInteger leftMask, NSUInteger rightMask,
                               BOOL& leftWasDown, BOOL& rightWasDown,
                               sf::Keyboard::Key leftKey, sf::Keyboard::Key rightKey,
                               sf::Keyboard::Scancode leftCode, sf::Keyboard::Scancode rightCode,
                               sf::priv::WindowImplCocoa& requester)
{
    processOneModifier(modifiers, leftMask,  leftWasDown,  leftKey,  leftCode,  requester);
    processOneModifier(modifiers, rightMask, rightWasDown, rightKey, rightCode, requester);
}



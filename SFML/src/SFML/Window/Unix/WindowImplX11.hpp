////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Laurent Gomila (laurent@sfml-dev.org)
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

#ifndef SFML_WINDOWIMPLX11_HPP
#define SFML_WINDOWIMPLX11_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Event.hpp>
#include <SFML/Window/WindowImpl.hpp>
#include <SFML/System/String.hpp>
#include <SFML/Window/WindowStyle.hpp> // Prevent conflict with macro None from Xlib
#include <X11/Xlib.h>
#include <deque>
#include <X11/extensions/Xrandr.h>


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
/// \brief Linux (X11) implementation of WindowImpl
///
////////////////////////////////////////////////////////////
class WindowImplX11 : public WindowImpl
{
public:

    ////////////////////////////////////////////////////////////
    /// \brief Construct the window implementation from an existing control
    ///
    /// \param handle Platform-specific handle of the control
    ///
    ////////////////////////////////////////////////////////////
    WindowImplX11(WindowHandle handle);

    ////////////////////////////////////////////////////////////
    /// \brief Create the window implementation
    ///
    /// \param mode  Video mode to use
    /// \param title Title of the window
    /// \param style Window style (resizable, fixed, or fullscren)
    /// \param settings Additional settings for the underlying OpenGL context
    ///
    ////////////////////////////////////////////////////////////
    WindowImplX11(VideoMode mode, const String& title, unsigned long style, const ContextSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~WindowImplX11();

    ////////////////////////////////////////////////////////////
    /// \brief Get the OS-specific handle of the window
    ///
    /// \return Handle of the window
    ///
    ////////////////////////////////////////////////////////////
    virtual WindowHandle getSystemHandle() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the position of the window
    ///
    /// \return Position of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual Vector2i getPosition() const;

    ////////////////////////////////////////////////////////////
    /// \brief Change the position of the window on screen
    ///
    /// \param position New position of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual void setPosition(const Vector2i& position);

    ////////////////////////////////////////////////////////////
    /// \brief Get the client size of the window
    ///
    /// \return Size of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual Vector2u getSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Change the size of the rendering region of the window
    ///
    /// \param size New size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual void setSize(const Vector2u& size);

    ////////////////////////////////////////////////////////////
    /// \brief Change the title of the window
    ///
    /// \param title New title
    ///
    ////////////////////////////////////////////////////////////
    virtual void setTitle(const String& title);

    ////////////////////////////////////////////////////////////
    /// \brief Change the window's icon
    ///
    /// \param width  Icon's width, in pixels
    /// \param height Icon's height, in pixels
    /// \param pixels Pointer to the pixels in memory, format must be RGBA 32 bits
    ///
    ////////////////////////////////////////////////////////////
    virtual void setIcon(unsigned int width, unsigned int height, const Uint8* pixels);

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the window
    ///
    /// \param visible True to show, false to hide
    ///
    ////////////////////////////////////////////////////////////
    virtual void setVisible(bool visible);

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the mouse cursor
    ///
    /// \param visible True to show, false to hide
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMouseCursorVisible(bool visible);

    ////////////////////////////////////////////////////////////
    /// \brief Grab or release the mouse cursor
    ///
    /// \param grabbed True to enable, false to disable
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMouseCursorGrabbed(bool grabbed);

    ////////////////////////////////////////////////////////////
    /// \brief Set the displayed cursor to a native system cursor
    ///
    /// \param cursor Native system cursor type to display
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMouseCursor(const CursorImpl& cursor);

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable automatic key-repeat
    ///
    /// \param enabled True to enable, false to disable
    ///
    ////////////////////////////////////////////////////////////
    virtual void setKeyRepeatEnabled(bool enabled);

    ////////////////////////////////////////////////////////////
    /// \brief Request the current window to be made the active
    ///        foreground window
    ///
    ////////////////////////////////////////////////////////////
    virtual void requestFocus();

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the window has the input focus
    ///
    /// \return True if window has focus, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    virtual bool hasFocus() const;

protected:

    ////////////////////////////////////////////////////////////
    /// \brief Process incoming events from the operating system
    ///
    ////////////////////////////////////////////////////////////
    virtual void processEvents();

private:

    ////////////////////////////////////////////////////////////
    /// \brief Request the WM to make the current window active
    ///
    ////////////////////////////////////////////////////////////
    void grabFocus();

    ////////////////////////////////////////////////////////////
    /// \brief Set fullscreen video mode
    ///
    /// \param Mode video mode to switch to
    ///
    ////////////////////////////////////////////////////////////
    void setVideoMode(const VideoMode& mode);

    ////////////////////////////////////////////////////////////
    /// \brief Reset to desktop video mode
    ///
    ////////////////////////////////////////////////////////////
    void resetVideoMode();

    ////////////////////////////////////////////////////////////
    /// \brief Switch to fullscreen mode
    ///
    ////////////////////////////////////////////////////////////
    void switchToFullscreen();

    ////////////////////////////////////////////////////////////
    /// \brief Set the WM protocols we support
    ///
    ////////////////////////////////////////////////////////////
    void setProtocols();

    ////////////////////////////////////////////////////////////
    /// \brief Update the last time we received user input
    ///
    /// \param time Last time we received user input
    ///
    ////////////////////////////////////////////////////////////
    void updateLastInputTime(::Time time);

    ////////////////////////////////////////////////////////////
    /// \brief Do some common initializations after the window has been created
    ///
    ////////////////////////////////////////////////////////////
    void initialize();

    ////////////////////////////////////////////////////////////
    /// \brief Create a transparent mouse cursor
    ///
    ////////////////////////////////////////////////////////////
    void createHiddenCursor();

    ////////////////////////////////////////////////////////////
    /// \brief Cleanup graphical resources attached to the window
    ///
    ////////////////////////////////////////////////////////////
    void cleanup();

    ////////////////////////////////////////////////////////////
    /// \brief Process an incoming event from the window
    ///
    /// \param windowEvent Event which has been received
    ///
    /// \return True if the event was processed, false if it was discarded
    ///
    ////////////////////////////////////////////////////////////
    bool processEvent(XEvent& windowEvent);

    ////////////////////////////////////////////////////////////
    /// \brief Check if a valid version of XRandR extension is present 
    ///
    /// \param xRandRMajor XRandR major version
    /// \param xRandRMinor XRandR minor version
    ///
    /// \return True if a valid XRandR version found, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    bool checkXRandR(int& xRandRMajor, int& xRandRMinor);

    ////////////////////////////////////////////////////////////
    /// \brief Get the RROutput of the primary monitor
    ///
    /// \param rootWindow the root window
    /// \param res screen resources
    /// \param xRandRMajor XRandR major version
    /// \param xRandRMinor XRandR minor version
    ///
    /// \return RROutput of the primary monitor
    ///
    ////////////////////////////////////////////////////////////
    RROutput getOutputPrimary(::Window& rootWindow, XRRScreenResources* res, int xRandRMajor, int xRandRMinor);

    ////////////////////////////////////////////////////////////
    /// \brief Get coordinates of the primary monitor
    ///
    /// \return Position of the primary monitor
    ///
    ////////////////////////////////////////////////////////////
    Vector2i getPrimaryMonitorPosition();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    ::Window   m_window;         ///< X identifier defining our window
    ::Display* m_display;        ///< Pointer to the display
    int        m_screen;         ///< Screen identifier
    XIM        m_inputMethod;    ///< Input method linked to the X display
    XIC        m_inputContext;   ///< Input context used to get unicode input in our window
    bool       m_isExternal;     ///< Tell whether the window has been created externally or by SFML
    RRMode     m_oldVideoMode;   ///< Video mode in use before we switch to fullscreen
    RRCrtc     m_oldRRCrtc;      ///< RRCrtc in use before we switch to fullscreen
    ::Cursor   m_hiddenCursor;   ///< As X11 doesn't provide cursor hiding, we must create a transparent one
    ::Cursor   m_lastCursor;     ///< Last cursor used -- this data is not owned by the window and is required to be always valid
    bool       m_keyRepeat;      ///< Is the KeyRepeat feature enabled?
    Vector2i   m_previousSize;   ///< Previous size of the window, to find if a ConfigureNotify event is a resize event (could be a move event only)
    bool       m_useSizeHints;   ///< Is the size of the window fixed with size hints?
    bool       m_fullscreen;     ///< Is the window in fullscreen?
    bool       m_cursorGrabbed;  ///< Is the mouse cursor trapped?
    bool       m_windowMapped;   ///< Has the window been mapped by the window manager?
    Pixmap     m_iconPixmap;     ///< The current icon pixmap if in use
    Pixmap     m_iconMaskPixmap; ///< The current icon mask pixmap if in use
    ::Time     m_lastInputTime;  ///< Last time we received user input
};

} // namespace priv

} // namespace sf


#endif // SFML_WINDOWIMPLX11_HPP

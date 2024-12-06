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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/WindowImpl.hpp> // included first to avoid a warning about macro redefinition
#include <SFML/Window/Win32/WglContext.hpp>
#include <SFML/System/ThreadLocalPtr.hpp>
#include <SFML/System/Lock.hpp>
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Err.hpp>
#include <sstream>
#include <vector>

// We check for this definition in order to avoid multiple definitions of GLAD
// entities during unity builds of SFML.
#ifndef SF_GLAD_WGL_IMPLEMENTATION_INCLUDED
#define SF_GLAD_WGL_IMPLEMENTATION_INCLUDED
#define SF_GLAD_WGL_IMPLEMENTATION
#include <glad/wgl.h>
#endif

namespace
{
    namespace WglContextImpl
    {
        // Some drivers are bugged and don't track the current HDC/HGLRC properly
        // In order to deactivate successfully, we need to track it ourselves as well
        sf::ThreadLocalPtr<sf::priv::WglContext> currentContext(NULL);


        ////////////////////////////////////////////////////////////
        void ensureInit()
        {
            static bool initialized = false;
            if (!initialized)
            {
                initialized = true;

                gladLoadWGL(NULL, sf::priv::WglContext::getFunction);
            }
        }


        ////////////////////////////////////////////////////////////
        void ensureExtensionsInit(HDC deviceContext)
        {
            static bool initialized = false;
            if (!initialized)
            {
                initialized = true;

                // We don't check the return value since the extension
                // flags are cleared even if loading fails
                gladLoadWGL(deviceContext, sf::priv::WglContext::getFunction);
            }
        }
    }
}


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
String getErrorString(DWORD errorCode)
{
    PTCHAR buffer;
    if (FormatMessage(FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, 0, reinterpret_cast<LPTSTR>(&buffer), 256, NULL) != 0)
    {
        String errMsg(buffer);
        LocalFree(buffer);
        return errMsg;
    }

    std::ostringstream ss;
    ss << "Error " << errorCode;
    return String(ss.str());
}


////////////////////////////////////////////////////////////
WglContext::WglContext(WglContext* shared) :
m_window       (NULL),
m_pbuffer      (NULL),
m_deviceContext(NULL),
m_context      (NULL),
m_ownsWindow   (false)
{
    WglContextImpl::ensureInit();

    // TODO: Delegate to the other constructor in C++11

    // Save the creation settings
    m_settings = ContextSettings();

    // Create the rendering surface (window or pbuffer if supported)
    createSurface(shared, 1, 1, VideoMode::getDesktopMode().bitsPerPixel);

    // Create the context
    createContext(shared);
}


////////////////////////////////////////////////////////////
WglContext::WglContext(WglContext* shared, const ContextSettings& settings, const WindowImpl* owner, unsigned int bitsPerPixel) :
m_window       (NULL),
m_pbuffer      (NULL),
m_deviceContext(NULL),
m_context      (NULL),
m_ownsWindow   (false)
{
    WglContextImpl::ensureInit();

    // Save the creation settings
    m_settings = settings;

    // Create the rendering surface from the owner window
    createSurface(owner->getSystemHandle(), bitsPerPixel);

    // Create the context
    createContext(shared);
}


////////////////////////////////////////////////////////////
WglContext::WglContext(WglContext* shared, const ContextSettings& settings, unsigned int width, unsigned int height) :
m_window       (NULL),
m_pbuffer      (NULL),
m_deviceContext(NULL),
m_context      (NULL),
m_ownsWindow   (false)
{
    WglContextImpl::ensureInit();

    // Save the creation settings
    m_settings = settings;

    // Create the rendering surface (window or pbuffer if supported)
    createSurface(shared, width, height, VideoMode::getDesktopMode().bitsPerPixel);

    // Create the context
    createContext(shared);
}


////////////////////////////////////////////////////////////
WglContext::~WglContext()
{
    // Notify unshared OpenGL resources of context destruction
    cleanupUnsharedResources();

    // Destroy the OpenGL context
    if (m_context)
    {
        if (WglContextImpl::currentContext == this)
        {
            if (wglMakeCurrent(m_deviceContext, NULL) == TRUE)
                WglContextImpl::currentContext = NULL;
        }

        wglDeleteContext(m_context);
    }

    // Destroy the device context
    if (m_deviceContext)
    {
        if (m_pbuffer)
        {
            wglReleasePbufferDCARB(m_pbuffer, m_deviceContext);
            wglDestroyPbufferARB(m_pbuffer);
        }
        else
        {
            ReleaseDC(m_window, m_deviceContext);
        }
    }

    // Destroy the window if we own it
    if (m_window && m_ownsWindow)
        DestroyWindow(m_window);
}


////////////////////////////////////////////////////////////
GlFunctionPointer WglContext::getFunction(const char* name)
{
    GlFunctionPointer address = reinterpret_cast<GlFunctionPointer>(wglGetProcAddress(reinterpret_cast<LPCSTR>(name)));

    if (address)
    {
        // Test whether the returned value is a valid error code
        ptrdiff_t errorCode = reinterpret_cast<ptrdiff_t>(address);

        if ((errorCode != -1) && (errorCode != 1) && (errorCode != 2) && (errorCode != 3))
            return address;
    }

    static HMODULE module = NULL;

    if (!module)
        module = GetModuleHandleA("OpenGL32.dll");

    if (module)
        return reinterpret_cast<GlFunctionPointer>(GetProcAddress(module, reinterpret_cast<LPCSTR>(name)));

    return 0;
}


////////////////////////////////////////////////////////////
bool WglContext::makeCurrent(bool current)
{
    if (!m_deviceContext || !m_context)
        return false;

    if (wglMakeCurrent(m_deviceContext, current ? m_context : NULL) == FALSE)
    {
        err() << "Failed to " << (current ? "activate" : "deactivate") << " OpenGL context: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
        return false;
    }

    WglContextImpl::currentContext = (current ? this : NULL);

    return true;
}


////////////////////////////////////////////////////////////
void WglContext::display()
{
    if (m_deviceContext && m_context)
        SwapBuffers(m_deviceContext);
}


////////////////////////////////////////////////////////////
void WglContext::setVerticalSyncEnabled(bool enabled)
{
    // Make sure that extensions are initialized
    WglContextImpl::ensureExtensionsInit(m_deviceContext);

    if (SF_GLAD_WGL_EXT_swap_control)
    {
        if (wglSwapIntervalEXT(enabled ? 1 : 0) == FALSE)
            err() << "Setting vertical sync failed: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
    }
    else
    {
        static bool warned = false;

        if (!warned)
        {
            // wglSwapIntervalEXT not supported
            err() << "Setting vertical sync not supported" << std::endl;

            warned = true;
        }
    }
}


////////////////////////////////////////////////////////////
int WglContext::selectBestPixelFormat(HDC deviceContext, unsigned int bitsPerPixel, const ContextSettings& settings, bool pbuffer)
{
    WglContextImpl::ensureInit();

    // Let's find a suitable pixel format -- first try with wglChoosePixelFormatARB
    int bestFormat = 0;
    if (SF_GLAD_WGL_ARB_pixel_format)
    {
        // Define the basic attributes we want for our window
        int intAttributes[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            0,                      0
        };

        // Let's check how many formats are supporting our requirements
        int  formats[512];
        UINT nbFormats = 0; // We must initialize to 0 otherwise broken drivers might fill with garbage in the following call
        bool isValid = wglChoosePixelFormatARB(deviceContext, intAttributes, NULL, 512, formats, &nbFormats) != FALSE;

        if (!isValid)
            err() << "Failed to enumerate pixel formats: " << getErrorString(GetLastError()).toAnsiString() << std::endl;

        // Get the best format among the returned ones
        if (isValid && (nbFormats > 0))
        {
            int bestScore = 0x7FFFFFFF;
            for (UINT i = 0; i < nbFormats; ++i)
            {
                // Extract the components of the current format
                int values[7];
                const int attributes[] =
                {
                    WGL_RED_BITS_ARB,
                    WGL_GREEN_BITS_ARB,
                    WGL_BLUE_BITS_ARB,
                    WGL_ALPHA_BITS_ARB,
                    WGL_DEPTH_BITS_ARB,
                    WGL_STENCIL_BITS_ARB,
                    WGL_ACCELERATION_ARB
                };

                if (wglGetPixelFormatAttribivARB(deviceContext, formats[i], PFD_MAIN_PLANE, 7, attributes, values) == FALSE)
                {
                    err() << "Failed to retrieve pixel format information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                    break;
                }

                int sampleValues[2] = {0, 0};
                if (SF_GLAD_WGL_ARB_multisample)
                {
                    const int sampleAttributes[] =
                    {
                        WGL_SAMPLE_BUFFERS_ARB,
                        WGL_SAMPLES_ARB
                    };

                    if (wglGetPixelFormatAttribivARB(deviceContext, formats[i], PFD_MAIN_PLANE, 2, sampleAttributes, sampleValues) == FALSE)
                    {
                        err() << "Failed to retrieve pixel format multisampling information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                        break;
                    }
                }

                int sRgbCapableValue = 0;
                if (SF_GLAD_WGL_ARB_framebuffer_sRGB || SF_GLAD_WGL_EXT_framebuffer_sRGB)
                {
                    const int sRgbCapableAttribute = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;

                    if (wglGetPixelFormatAttribivARB(deviceContext, formats[i], PFD_MAIN_PLANE, 1, &sRgbCapableAttribute, &sRgbCapableValue) == FALSE)
                    {
                        err() << "Failed to retrieve pixel format sRGB capability information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                        break;
                    }
                }

                if (pbuffer)
                {
                    const int pbufferAttributes[] =
                    {
                        WGL_DRAW_TO_PBUFFER_ARB
                    };

                    int pbufferValue = 0;

                    if (wglGetPixelFormatAttribivARB(deviceContext, formats[i], PFD_MAIN_PLANE, 1, pbufferAttributes, &pbufferValue) == FALSE)
                    {
                        err() << "Failed to retrieve pixel format pbuffer information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                        break;
                    }

                    if (pbufferValue != GL_TRUE)
                        continue;
                }

                // Evaluate the current configuration
                int color = values[0] + values[1] + values[2] + values[3];
                int score = evaluateFormat(bitsPerPixel, settings, color, values[4], values[5], sampleValues[0] ? sampleValues[1] : 0, values[6] == WGL_FULL_ACCELERATION_ARB, sRgbCapableValue == TRUE);

                // Keep it if it's better than the current best
                if (score < bestScore)
                {
                    bestScore  = score;
                    bestFormat = formats[i];
                }
            }
        }
    }

    // ChoosePixelFormat doesn't support pbuffers
    if (pbuffer)
        return bestFormat;

    // Find a pixel format with ChoosePixelFormat, if wglChoosePixelFormatARB is not supported
    if (bestFormat == 0)
    {
        // Setup a pixel format descriptor from the rendering settings
        PIXELFORMATDESCRIPTOR descriptor;
        ZeroMemory(&descriptor, sizeof(descriptor));
        descriptor.nSize        = sizeof(descriptor);
        descriptor.nVersion     = 1;
        descriptor.iLayerType   = PFD_MAIN_PLANE;
        descriptor.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        descriptor.iPixelType   = PFD_TYPE_RGBA;
        descriptor.cColorBits   = static_cast<BYTE>(bitsPerPixel);
        descriptor.cDepthBits   = static_cast<BYTE>(settings.depthBits);
        descriptor.cStencilBits = static_cast<BYTE>(settings.stencilBits);
        descriptor.cAlphaBits   = bitsPerPixel == 32 ? 8 : 0;

        // Get the pixel format that best matches our requirements
        bestFormat = ChoosePixelFormat(deviceContext, &descriptor);
    }

    return bestFormat;
}


////////////////////////////////////////////////////////////
void WglContext::setDevicePixelFormat(unsigned int bitsPerPixel)
{
    int bestFormat = selectBestPixelFormat(m_deviceContext, bitsPerPixel, m_settings);

    if (bestFormat == 0)
    {
        err() << "Failed to find a suitable pixel format for device context: " << getErrorString(GetLastError()).toAnsiString() << std::endl
              << "Cannot create OpenGL context" << std::endl;
        return;
    }

    // Extract the depth and stencil bits from the chosen format
    PIXELFORMATDESCRIPTOR actualFormat;
    actualFormat.nSize    = sizeof(actualFormat);
    actualFormat.nVersion = 1;
    DescribePixelFormat(m_deviceContext, bestFormat, sizeof(actualFormat), &actualFormat);

    // Set the chosen pixel format
    if (SetPixelFormat(m_deviceContext, bestFormat, &actualFormat) == FALSE)
    {
        err() << "Failed to set pixel format for device context: " << getErrorString(GetLastError()).toAnsiString() << std::endl
              << "Cannot create OpenGL context" << std::endl;
        return;
    }
}


////////////////////////////////////////////////////////////
void WglContext::updateSettingsFromPixelFormat()
{
    int format = GetPixelFormat(m_deviceContext);

    if (format == 0)
    {
        err() << "Failed to get selected pixel format: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
        return;
    }

    PIXELFORMATDESCRIPTOR actualFormat;
    actualFormat.nSize    = sizeof(actualFormat);
    actualFormat.nVersion = 1;

    if (DescribePixelFormat(m_deviceContext, format, sizeof(actualFormat), &actualFormat) == 0)
    {
        err() << "Failed to retrieve pixel format information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
        return;
    }

    if (SF_GLAD_WGL_ARB_pixel_format)
    {
        const int attributes[] = {WGL_DEPTH_BITS_ARB, WGL_STENCIL_BITS_ARB};
        int values[2];

        if (wglGetPixelFormatAttribivARB(m_deviceContext, format, PFD_MAIN_PLANE, 2, attributes, values) == TRUE)
        {
            m_settings.depthBits   = static_cast<unsigned int>(values[0]);
            m_settings.stencilBits = static_cast<unsigned int>(values[1]);
        }
        else
        {
            err() << "Failed to retrieve pixel format information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
            m_settings.depthBits   = actualFormat.cDepthBits;
            m_settings.stencilBits = actualFormat.cStencilBits;
        }

        if (SF_GLAD_WGL_ARB_multisample)
        {
            const int sampleAttributes[] = {WGL_SAMPLE_BUFFERS_ARB, WGL_SAMPLES_ARB};
            int sampleValues[2];

            if (wglGetPixelFormatAttribivARB(m_deviceContext, format, PFD_MAIN_PLANE, 2, sampleAttributes, sampleValues) == TRUE)
            {
                m_settings.antialiasingLevel = static_cast<unsigned int>(sampleValues[0] ? sampleValues[1] : 0);
            }
            else
            {
                err() << "Failed to retrieve pixel format multisampling information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                m_settings.antialiasingLevel = 0;
            }
        }
        else
        {
            m_settings.antialiasingLevel = 0;
        }

        if (SF_GLAD_WGL_ARB_framebuffer_sRGB || SF_GLAD_WGL_EXT_framebuffer_sRGB)
        {
            const int sRgbCapableAttribute = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;
            int sRgbCapableValue = 0;

            if (wglGetPixelFormatAttribivARB(m_deviceContext, format, PFD_MAIN_PLANE, 1, &sRgbCapableAttribute, &sRgbCapableValue) == TRUE)
            {
                m_settings.sRgbCapable = (sRgbCapableValue == TRUE);
            }
            else
            {
                err() << "Failed to retrieve pixel format sRGB capability information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                m_settings.sRgbCapable = false;
            }
        }
        else
        {
            m_settings.sRgbCapable = false;
        }
    }
    else
    {
        m_settings.depthBits   = actualFormat.cDepthBits;
        m_settings.stencilBits = actualFormat.cStencilBits;
        m_settings.antialiasingLevel = 0;
    }
}


////////////////////////////////////////////////////////////
void WglContext::createSurface(WglContext* shared, unsigned int width, unsigned int height, unsigned int bitsPerPixel)
{
    // Check if the shared context already exists and pbuffers are supported
    if (shared && shared->m_deviceContext && SF_GLAD_WGL_ARB_pbuffer)
    {
        int bestFormat = selectBestPixelFormat(shared->m_deviceContext, bitsPerPixel, m_settings, true);

        if (bestFormat > 0)
        {
            int attributes[] = {0, 0};

            m_pbuffer = wglCreatePbufferARB(shared->m_deviceContext, bestFormat, static_cast<int>(width), static_cast<int>(height), attributes);

            if (m_pbuffer)
            {
                m_window = shared->m_window;
                m_deviceContext = wglGetPbufferDCARB(m_pbuffer);

                if (!m_deviceContext)
                {
                    err() << "Failed to retrieve pixel buffer device context: " << getErrorString(GetLastError()).toAnsiString() << std::endl;

                    wglDestroyPbufferARB(m_pbuffer);
                    m_pbuffer = NULL;
                }
            }
            else
            {
                err() << "Failed to create pixel buffer: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
            }
        }
    }

    // If pbuffers are not available we use a hidden window as the off-screen surface to draw to
    if (!m_deviceContext)
    {
        // We can't create a memory DC, the resulting context wouldn't be compatible
        // with other contexts and thus wglShareLists would always fail

        // Create the hidden window
        m_window = CreateWindowA("STATIC", "", WS_POPUP | WS_DISABLED, 0, 0, static_cast<int>(width), static_cast<int>(height), NULL, NULL, GetModuleHandle(NULL), NULL);
        ShowWindow(m_window, SW_HIDE);
        m_deviceContext = GetDC(m_window);

        m_ownsWindow = true;

        // Set the pixel format of the device context
        setDevicePixelFormat(bitsPerPixel);
    }

    // Update context settings from the selected pixel format
    updateSettingsFromPixelFormat();
}


////////////////////////////////////////////////////////////
void WglContext::createSurface(HWND window, unsigned int bitsPerPixel)
{
    m_window = window;
    m_deviceContext = GetDC(window);

    // Set the pixel format of the device context
    setDevicePixelFormat(bitsPerPixel);

    // Update context settings from the selected pixel format
    updateSettingsFromPixelFormat();
}


////////////////////////////////////////////////////////////
void WglContext::createContext(WglContext* shared)
{
    // We can't create an OpenGL context if we don't have a DC
    if (!m_deviceContext)
        return;

    // Get a working copy of the context settings
    ContextSettings settings = m_settings;

    // Get the context to share display lists with
    HGLRC sharedContext = shared ? shared->m_context : NULL;

    // Create the OpenGL context -- first try using wglCreateContextAttribsARB
    while (!m_context && m_settings.majorVersion)
    {
        if (SF_GLAD_WGL_ARB_create_context)
        {
            std::vector<int> attributes;

            // Check if the user requested a specific context version (anything > 1.1)
            if ((m_settings.majorVersion > 1) || ((m_settings.majorVersion == 1) && (m_settings.minorVersion > 1)))
            {
                attributes.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);
                attributes.push_back(static_cast<int>(m_settings.majorVersion));
                attributes.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);
                attributes.push_back(static_cast<int>(m_settings.minorVersion));
            }

            // Check if setting the profile is supported
            if (SF_GLAD_WGL_ARB_create_context_profile)
            {
                int profile = (m_settings.attributeFlags & ContextSettings::Core) ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
                int debug = (m_settings.attributeFlags & ContextSettings::Debug) ? WGL_CONTEXT_DEBUG_BIT_ARB : 0;

                attributes.push_back(WGL_CONTEXT_PROFILE_MASK_ARB);
                attributes.push_back(profile);
                attributes.push_back(WGL_CONTEXT_FLAGS_ARB);
                attributes.push_back(debug);
            }
            else
            {
                if ((m_settings.attributeFlags & ContextSettings::Core) || (m_settings.attributeFlags & ContextSettings::Debug))
                    err() << "Selecting a profile during context creation is not supported,"
                          << "disabling comptibility and debug" << std::endl;

                m_settings.attributeFlags = ContextSettings::Default;
            }

            // Append the terminating 0
            attributes.push_back(0);
            attributes.push_back(0);

            if (sharedContext)
            {
                static Mutex mutex;
                Lock lock(mutex);

                if (WglContextImpl::currentContext == shared)
                {
                    if (wglMakeCurrent(shared->m_deviceContext, NULL) == FALSE)
                    {
                        err() << "Failed to deactivate shared context before sharing: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                        return;
                    }

                    WglContextImpl::currentContext = NULL;
                }
            }

            // Create the context
            m_context = wglCreateContextAttribsARB(m_deviceContext, sharedContext, &attributes[0]);
        }
        else
        {
            // If wglCreateContextAttribsARB is not supported, there is no need to keep trying
            break;
        }

        // If we couldn't create the context, first try disabling flags,
        // then lower the version number and try again -- stop at 0.0
        // Invalid version numbers will be generated by this algorithm (like 3.9), but we really don't care
        if (!m_context)
        {
            if (m_settings.attributeFlags != ContextSettings::Default)
            {
                m_settings.attributeFlags = ContextSettings::Default;
            }
            else if (m_settings.minorVersion > 0)
            {
                // If the minor version is not 0, we decrease it and try again
                m_settings.minorVersion--;

                m_settings.attributeFlags = settings.attributeFlags;
            }
            else
            {
                // If the minor version is 0, we decrease the major version
                m_settings.majorVersion--;
                m_settings.minorVersion = 9;

                m_settings.attributeFlags = settings.attributeFlags;
            }
        }
    }

    // If wglCreateContextAttribsARB failed, use wglCreateContext
    if (!m_context)
    {
        // set the context version to 1.1 (arbitrary) and disable flags
        m_settings.majorVersion = 1;
        m_settings.minorVersion = 1;
        m_settings.attributeFlags = ContextSettings::Default;

        m_context = wglCreateContext(m_deviceContext);
        if (!m_context)
        {
            err() << "Failed to create an OpenGL context for this window: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
            return;
        }

        // Share this context with others
        if (sharedContext)
        {
            // wglShareLists doesn't seem to be thread-safe
            static Mutex mutex;
            Lock lock(mutex);

            if (WglContextImpl::currentContext == shared)
            {
                if (wglMakeCurrent(shared->m_deviceContext, NULL) == FALSE)
                {
                    err() << "Failed to deactivate shared context before sharing: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                    return;
                }

                WglContextImpl::currentContext = NULL;
            }

            if (wglShareLists(sharedContext, m_context) == FALSE)
                err() << "Failed to share the OpenGL context: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
        }
    }

    // If we are the shared context, initialize extensions now
    // This enables us to re-create the shared context using extensions if we need to
    if (!shared && m_context)
    {
        makeCurrent(true);
        WglContextImpl::ensureExtensionsInit(m_deviceContext);
        makeCurrent(false);
    }
}

} // namespace priv

} // namespace sf

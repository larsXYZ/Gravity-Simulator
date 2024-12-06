/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus' Graphical User Interface
// Copyright (C) 2012-2024 Bruno Van de Velde (vdv_b@tgui.eu)
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TGUI_BACKEND_SDL_TTF_GLES2_INCLUDE_HPP
#define TGUI_BACKEND_SDL_TTF_GLES2_INCLUDE_HPP

#include <TGUI/Config.hpp>

#ifdef TGUI_NEXT
    #if !TGUI_HAS_BACKEND_SDL_TTF_GLES2
        #error "TGUI wasn't build with the SDL_TTF_GLES2 backend"
    #endif
#else
    // Allow this file to be used on Android even when Config.hpp claims the backend isn't part of TGUI.
    // This is needed when ndk-build is used instead of CMake.
    #if !TGUI_HAS_BACKEND_SDL_TTF_GLES2 && !defined(TGUI_SYSTEM_ANDROID)
        #error "TGUI wasn't build with the SDL_TTF_GLES2 backend"
    #endif

    // TODO: Warning if defined(TGUI_SYSTEM_ANDROID) && !TGUI_HAS_BACKEND_SDL_TTF_GLES2
    //       This situation will no longer be supported once we drop support for ndk-build and require the use of CMake
#endif

#if !TGUI_BUILD_AS_CXX_MODULE
    #include <TGUI/Backend/Window/SDL/BackendSDL.hpp>
    #include <TGUI/Backend/Renderer/GLES2/BackendRendererGLES2.hpp>
    #include <TGUI/Backend/Font/SDL_ttf/BackendFontSDLttf.hpp>
#endif

TGUI_MODULE_EXPORT namespace tgui
{
    inline namespace SDL_TTF_GLES2
    {
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Gui class for SDL + GLES2 backend (using SDL_ttf for fonts)
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        class TGUI_API Gui : public BackendGuiSDL
        {
        public:

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /// @brief Default constructor
            ///
            /// @warning You must still call setWindow on the Gui before using any TGUI functions
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            Gui() = default;

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /// @brief Constructor that immediately sets the window on which the gui should be drawn
            ///
            /// @param window  The SDL window that will be used by the gui
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            Gui(SDL_Window* window)
            {
                setWindow(window);
            }

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /// @brief Sets the window on which the gui should be drawn
            ///
            /// @param window  The SDL window that will be used by the gui
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            void setWindow(SDL_Window* window);

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        protected:

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /// @brief Shows the drawn content with SDL_GL_SwapWindow
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            void presentScreen() override;

#if ((SDL_MAJOR_VERSION == 2) && (SDL_MINOR_VERSION < 26))
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /// @brief Updates m_framebufferSize by calling SDL_GL_GetDrawableSize
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            void updateFramebufferSize() override;
#endif
        };
    }
}

#endif // TGUI_BACKEND_SDL_TTF_GLES2_INCLUDE_HPP

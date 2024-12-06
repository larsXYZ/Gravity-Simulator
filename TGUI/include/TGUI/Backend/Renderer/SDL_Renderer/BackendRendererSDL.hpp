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

#ifndef TGUI_BACKEND_RENDERER_SDL_HPP
#define TGUI_BACKEND_RENDERER_SDL_HPP

#include <TGUI/Backend/Renderer/SDL_Renderer/BackendTextureSDL.hpp>
#include <TGUI/Backend/Renderer/SDL_Renderer/BackendRenderTargetSDL.hpp>
#include <TGUI/Backend/Renderer/SDL_Renderer/CanvasSDL.hpp>

#if !TGUI_BUILD_AS_CXX_MODULE
    #include <TGUI/Backend/Renderer/BackendRenderer.hpp>
#endif

#if !TGUI_EXPERIMENTAL_USE_STD_MODULE
    #include <memory>
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !TGUI_BUILD_AS_CXX_MODULE
struct SDL_Renderer;
#endif

TGUI_MODULE_EXPORT namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Backend renderer that uses SDL_Renderer
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class TGUI_API BackendRendererSDL : public BackendRenderer
    {
    public:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Initializes the backend renderer
        ///
        /// @param renderer  SDL_Renderer object that should be used for rendering
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        BackendRendererSDL(SDL_Renderer* renderer);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Creates a new empty texture object
        /// @return Shared pointer to a new texture object
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD std::shared_ptr<BackendTexture> createTexture() override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Get the maximum allowed texture size
        ///
        /// @return Maximum width and height that you should try to use in a single texture
        ///
        /// This maximum size is defined by the graphics driver. Most likely this will return 8192 or 16384.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD unsigned int getMaximumTextureSize() override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns a pointer to the SDL_Renderer object that was used to create this backend renderer
        ///
        /// @return SDL_Renderer pointer that was passed to the constructor of this object
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD SDL_Renderer* getInternalRenderer() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    private:

        SDL_Renderer* m_renderer = nullptr;
        unsigned int m_maxTextureSize = 0;
    };
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TGUI_BACKEND_RENDERER_SDL_HPP

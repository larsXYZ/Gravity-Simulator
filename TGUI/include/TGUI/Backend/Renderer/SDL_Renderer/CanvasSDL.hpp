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

#ifndef TGUI_CANVAS_SDL_HPP
#define TGUI_CANVAS_SDL_HPP

#include <TGUI/Backend/Renderer/SDL_Renderer/BackendTextureSDL.hpp>

#if !TGUI_BUILD_AS_CXX_MODULE
    #include <TGUI/Backend/Renderer/BackendRenderTarget.hpp>
    #include <TGUI/Widgets/CanvasBase.hpp>
#endif

#include <TGUI/extlibs/IncludeSDL.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGUI_MODULE_EXPORT namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief CanvasSDL provides a way to directly render SDL contents on a widget
    ///
    /// When gui.draw() is called, all widgets are drawn at once. If you wish to have custom SDL rendering inbetween
    /// TGUI widgets (e.g. draw to the background of a child window) then you need to use a CanvasSDL widget.
    ///
    /// The canvas widget is essentially just a wrapper around a render target texture. You draw your SDL contents on top of the
    /// canvas instead of on the window. The canvas is then added to the gui between the widgets where you want the rendering
    /// to appear.
    ///
    /// You can redraw the contents of the canvas at any time, but make sure to always start by calling SDL_SetRenderTarget
    /// with the texture of the canvas and end the rendering by calling SDL_SetRenderTarget with a nullptr (or other target).
    ///
    /// Example:
    /// @code
    /// SDL_Texture* imgTexture;
    ///
    /// auto canvas = tgui::CanvasSDL::create({400, 300});
    /// gui.add(canvas);
    ///
    /// SDL_SetRenderTarget(renderer, canvas->getTextureTarget());  // Let drawing happen on the canvas instead of the window
    /// SDL_RenderClear(renderer);                                  // Clear the contents of the canvas
    /// SDL_RenderCopy(renderer, imgTexture, nullptr, nullptr);     // Draw an image to the canvas
    /// SDL_SetRenderTarget(renderer, nullptr);                     // Let further drawing happen on the window again
    /// @endcode
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class TGUI_API CanvasSDL : public CanvasBase
    {
    public:

        using Ptr = std::shared_ptr<CanvasSDL>; //!< Shared widget pointer
        using ConstPtr = std::shared_ptr<const CanvasSDL>; //!< Shared constant widget pointer

        static constexpr const char StaticWidgetType[] = "CanvasSDL"; //!< Type name of the widget

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /// @brief Constructor
        /// @param typeName     Type of the widget
        /// @param initRenderer Should the renderer be initialized? Should be true unless a derived class initializes it.
        /// @see create
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        CanvasSDL(const char* typeName = StaticWidgetType, bool initRenderer = true);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Copy constructor
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        CanvasSDL(const CanvasSDL& copy);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Default move constructor
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        CanvasSDL(CanvasSDL&& copy) = default;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Overload of copy assignment operator
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        CanvasSDL& operator= (const CanvasSDL& right);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Default move assignment operator
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        CanvasSDL& operator= (CanvasSDL&& right) = default;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Creates a new canvas widget
        ///
        /// @param size  Size of the canvas
        ///
        /// @return The new canvas
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD static CanvasSDL::Ptr create(const Layout2d& size = {"100%", "100%"});

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Makes a copy of another canvas
        ///
        /// @param canvas  The other canvas
        ///
        /// @return The new canvas
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD static CanvasSDL::Ptr copy(const CanvasSDL::ConstPtr& canvas);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the size of the widget
        ///
        /// @param size  The new size of the widget
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setSize(const Layout2d& size) override;
        using Widget::setSize;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Draw the widget to a render target
        ///
        /// @param target Render target to draw to
        /// @param states Current render states
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void draw(BackendRenderTarget& target, RenderStates states) const override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Access the internal texture on which you can render
        ///
        /// @return Pointer to the internal texture
        ///
        /// The texture has a size that is equal or larger than the size of the canvas widget.
        /// The pixel format of the texture is SDL_PIXELFORMAT_RGBA32.
        /// The texture was created with SDL_TEXTUREACCESS_TARGET access.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD SDL_Texture* getTextureTarget()
        {
            return m_textureTarget;
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Makes a copy of the widget
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD Widget::Ptr clone() const override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        SDL_Texture* m_textureTarget = nullptr;
        Vector2u m_textureSize;
        Vector2u m_usedTextureSize;
        std::shared_ptr<BackendTextureSDL> m_backendTexture;
    };
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TGUI_CANVAS_SDL_HPP

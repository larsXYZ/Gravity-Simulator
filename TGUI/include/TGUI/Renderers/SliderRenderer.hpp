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

#ifndef TGUI_SLIDER_RENDERER_HPP
#define TGUI_SLIDER_RENDERER_HPP

#include <TGUI/Renderers/WidgetRenderer.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGUI_MODULE_EXPORT namespace tgui
{
    class TGUI_API SliderRenderer : public WidgetRenderer
    {
    public:

        using WidgetRenderer::WidgetRenderer;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the size of the borders
        ///
        /// @param borders  Size of the borders
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setBorders(const Borders& borders);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the size of the borders
        ///
        /// @return border size
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD Borders getBorders() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the color of the track
        ///
        /// @param color  The new track color
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTrackColor(Color color);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the color of the track
        ///
        /// @return Track color
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD Color getTrackColor() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the color of the track in hover state (when the mouse is on top of it)
        ///
        /// @param color  The new hover track color
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTrackColorHover(Color color);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the color of the track in hover state (when the mouse is on top of it)
        ///
        /// @return Hover track color
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD Color getTrackColorHover() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the color of the thumb
        ///
        /// @param color  The new thumb color
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setThumbColor(Color color);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the color of the thumb
        ///
        /// @return Thumb color
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD Color getThumbColor() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the color of the thumb in hover state (when the mouse is on top of it)
        ///
        /// @param color  The new hover thumb color
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setThumbColorHover(Color color);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the color of the thumb in hover state (when the mouse is on top of it)
        ///
        /// @return Hover thumb color
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD Color getThumbColorHover() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the color of the borders
        ///
        /// @param color  New border color
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setBorderColor(Color color);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the color of the borders
        ///
        /// @return Border color
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD Color getBorderColor() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the color of the borders in the hover state (when the mouse is on top of the slider)
        ///
        /// @param color  New border color
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setBorderColorHover(Color color);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the color of the borders in the hover state (when the mouse is on top of the slider)
        ///
        /// @return Border color
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD Color getBorderColorHover() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the image of the track
        ///
        /// @param texture  The new track texture
        ///
        /// When this image is set, the track color property will be ignored.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTextureTrack(const Texture& texture);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the image of the track
        ///
        /// @return Track texture
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD const Texture& getTextureTrack() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the image of the track that is displayed when the mouse is on top of it
        ///
        /// @param texture  The new hover track texture
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTextureTrackHover(const Texture& texture);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the image of the track that is displayed when the mouse is on top of it
        ///
        /// @return Hover track texture
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD const Texture& getTextureTrackHover() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the image of the thumb
        ///
        /// @param texture  The new thumb texture
        ///
        /// When this image is set, the thumb color property will be ignored.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTextureThumb(const Texture& texture);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the image of the thumb
        ///
        /// @return Thumb texture
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD const Texture& getTextureThumb() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the image of the thumb that is displayed when the mouse is on top of it
        ///
        /// @param texture  The new hover thumb texture
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTextureThumbHover(const Texture& texture);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the image of the thumb that is displayed when the mouse is on top of it
        ///
        /// @return Hover thumb texture
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD const Texture& getTextureThumbHover() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes whether the center of the thumb or the sides of the thumb must remain on top of the track.
        ///
        /// @param keepThumbInside  Should the sides of the thumb align with the sides of the track at min and max values?
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setThumbWithinTrack(bool keepThumbInside);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns whether the center of the thumb or the sides of the thumb must remain on top of the track.
        ///
        /// @return Should the sides of the thumb align with the sides of the track at min and max values?
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD bool getThumbWithinTrack() const;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TGUI_SLIDER_RENDERER_HPP
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

#ifndef TGUI_BACKEND_FONT_RAYLIB_HPP
#define TGUI_BACKEND_FONT_RAYLIB_HPP

#include <TGUI/Config.hpp>
#if !TGUI_BUILD_AS_CXX_MODULE
    #include <TGUI/Backend/Font/BackendFont.hpp>

    struct GlyphInfo;
#endif

#if !TGUI_EXPERIMENTAL_USE_STD_MODULE
    #include <unordered_map>
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGUI_MODULE_EXPORT namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Font implementations that uses Raylib to load glyphs
    /// @since TGUI 1.3
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class TGUI_API BackendFontRaylib : public BackendFont
    {
    public:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Loads a font from memory
        ///
        /// @param data         Moved pointer to the file data in memory
        /// @param sizeInBytes  Size of the data to load, in bytes
        ///
        /// @return True if the font was loaded successfully, false otherwise
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool loadFromMemory(std::unique_ptr<std::uint8_t[]> data, std::size_t sizeInBytes) override;
        using BackendFont::loadFromMemory;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns whether a font contains a certain glyph
        ///
        /// @param codePoint  Character to check
        ///
        /// @return Does the font contain this character?
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD bool hasGlyph(char32_t codePoint) const override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Retrieve a glyph of the font
        ///
        /// If the font is a bitmap font, not all character sizes might be available. If the glyph is not available at the
        /// requested size, an empty glyph is returned.
        ///
        /// @param codePoint        Unicode code point of the character to get
        /// @param characterSize    Reference character size
        /// @param bold             Retrieve the bold version or the regular one?
        /// @param outlineThickness Thickness of outline (when != 0 the glyph will not be filled)
        ///
        /// @return The glyph corresponding to codePoint and characterSize
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD FontGlyph getGlyph(char32_t codePoint, unsigned int characterSize, bool bold, float outlineThickness = 0) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the kerning offset of two glyphs
        ///
        /// The kerning is an extra offset (negative) to apply between two glyphs when rendering them, to make the pair look
        /// more "natural". For example, the pair "AV" have a special kerning to make them closer than other characters.
        /// Most of the glyphs pairs have a kerning offset of zero, though.
        ///
        /// @param first         Unicode code point of the first character
        /// @param second        Unicode code point of the second character
        /// @param characterSize Size of the characters
        /// @param bold          Are the glyphs bold or regular?
        ///
        /// @return Kerning value for first and second, in pixels
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD float getKerning(char32_t first, char32_t second, unsigned int characterSize, bool bold) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the line spacing
        ///
        /// Line spacing is the vertical offset to apply between two consecutive lines of text.
        ///
        /// @param characterSize Size of the characters
        ///
        /// @return Line spacing, in pixels
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD float getLineSpacing(unsigned int characterSize) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the height required to render a line of text
        ///
        /// @param characterSize  Size of the characters
        ///
        /// @return Sum of font ascent and descent
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD float getFontHeight(unsigned int characterSize) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the maximum height of a glyph above the baseline
        ///
        /// @param characterSize  Size of the characters
        ///
        /// @return Ascent of the font
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD float getAscent(unsigned int characterSize) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the maximum height of a glyph below the baseline
        ///
        /// @param characterSize  Size of the characters
        ///
        /// @return Descent of the font
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD float getDescent(unsigned int characterSize) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Get the position of the underline
        ///
        /// Underline position is the vertical offset to apply between the baseline and the underline.
        ///
        /// @param characterSize  Reference character size
        ///
        /// @return Underline position, in pixels
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD float getUnderlinePosition(unsigned int characterSize) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Get the thickness of the underline
        ///
        /// Underline thickness is the vertical size of the underline.
        ///
        /// @param characterSize  Reference character size
        ///
        /// @return Underline thickness, in pixels
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD float getUnderlineThickness(unsigned int characterSize) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the texture that is used to store glyphs of the given character size
        ///
        /// @param characterSize  Size of the characters that should be part of the texture
        /// @param textureVersion Counter that is incremented each time the texture is changed, returned by this function
        ///
        /// @return Texture to render text glyphs with
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD std::shared_ptr<BackendTexture> getTexture(unsigned int characterSize, unsigned int& textureVersion) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the size of the texture that is used to store glyphs of the given character size
        ///
        /// @param characterSize  Size of the characters that should be part of the texture
        ///
        /// @return Size of the texture that holds the glyphs
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD Vector2u getTextureSize(unsigned int characterSize) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Enable or disable the smooth filter
        ///
        /// When the filter is activated, the font appears smoother so that pixels are less noticeable. However if you want
        /// the font to look exactly the same as its source file then you should disable it.
        /// The smooth filter is enabled by default.
        ///
        /// @param smooth  True to enable smoothing, false to disable it
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setSmooth(bool smooth) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /// @brief Sets the scale at which to render each glyph, to allow rendering text at a higher resolution
        ///
        /// Text may be rendered blurry if the font scale doesn't match the ratio between the window size and gui view.
        /// Default scaling is 1. If the scale is set to 1.5 then a font with text size 20 would be internally rendered at 30px.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setFontScale(float scale) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Converts and caches the loaded glyph
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        FontGlyph loadGlyph(const GlyphInfo& glyphInfo, char32_t codePoint, unsigned int scaledTextSize, bool bold, float scaledOutlineThickness);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Reserves space in the texture to place the glyph
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGUI_NODISCARD UIntRect findAvailableGlyphRect(unsigned int width, unsigned int height);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Text size in TGUI and FreeType are based on the ascent of the text. Raylib uses stb_truetype and specifically the
        // stbtt_ScaleForPixelHeight function, which considers the font size to be ascent + descent.
        // This function estimates which font size we need to give to raylib to get the expected result for our text size.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        int estimateFontSize(unsigned int scaledTextSize);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        struct Row
        {
            Row(unsigned int rowTop, unsigned int rowHeight) : width(0), top(rowTop), height(rowHeight) {}

            unsigned int width;  //!< Current width of the row
            unsigned int top;    //!< Y position of the row into the texture
            unsigned int height; //!< Height of the row
        };

        std::unordered_map<std::uint64_t, FontGlyph> m_glyphs;
        unsigned int     m_nextRow = 3; //!< Y position of the next new row in the texture (first 2 rows contain pixels for underlining)
        std::vector<Row> m_rows;

        std::unique_ptr<std::uint8_t[]> m_fileContents;
        std::size_t m_fileSize = 0;

        std::unique_ptr<std::uint8_t[]> m_pixels;
        std::shared_ptr<BackendTexture> m_texture;
        unsigned int m_textureSize = 0;
        unsigned int m_textureVersion = 0;

        std::unordered_map<unsigned int, int> m_cachedAscents;      // text size -> font ascent
        std::unordered_map<unsigned int, int> m_correctedTextSizes; // text size (ascent) -> raylib text size (ascent + descent)
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TGUI_BACKEND_FONT_RAYLIB_HPP

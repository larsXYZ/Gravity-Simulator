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

#ifndef SFML_RENDERTEXTUREIMPLDEFAULT_HPP
#define SFML_RENDERTEXTUREIMPLDEFAULT_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/RenderTextureImpl.hpp>
#include <SFML/Window/GlResource.hpp>
#include <SFML/Window/Context.hpp>


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
/// \brief Default specialization of RenderTextureImpl,
///        using an in-memory context
///
////////////////////////////////////////////////////////////
class RenderTextureImplDefault : public RenderTextureImpl, GlResource
{
public:

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    RenderTextureImplDefault();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~RenderTextureImplDefault();

    ////////////////////////////////////////////////////////////
    /// \brief Get the maximum anti-aliasing level supported by the system
    ///
    /// \return The maximum anti-aliasing level supported by the system
    ///
    ////////////////////////////////////////////////////////////
    static unsigned int getMaximumAntialiasingLevel();

private:

    ////////////////////////////////////////////////////////////
    /// \brief Create the render texture implementation
    ///
    /// \param width      Width of the texture to render to
    /// \param height     Height of the texture to render to
    /// \param textureId  OpenGL identifier of the target texture
    /// \param settings   Context settings to create render-texture with
    ///
    /// \return True if creation has been successful
    ///
    ////////////////////////////////////////////////////////////
    virtual bool create(unsigned int width, unsigned int height, unsigned int textureId, const ContextSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief Activate or deactivate the render texture for rendering
    ///
    /// \param active True to activate, false to deactivate
    ///
    /// \return True on success, false on failure
    ///
    ////////////////////////////////////////////////////////////
    virtual bool activate(bool active);

    ////////////////////////////////////////////////////////////
    /// \brief Tell if the render-texture will use sRGB encoding when drawing on it
    ///
    /// You can request sRGB encoding for a render-texture
    /// by having the sRgbCapable flag set for the context parameter of create() method
    ///
    /// \return True if the render-texture use sRGB encoding, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    virtual bool isSrgb() const;

    ////////////////////////////////////////////////////////////
    /// \brief Update the pixels of the target texture
    ///
    /// \param textureId OpenGL identifier of the target texture
    ///
    ////////////////////////////////////////////////////////////
    virtual void updateTexture(unsigned textureId);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Context*     m_context; //!< P-Buffer based context
    unsigned int m_width;   //!< Width of the P-Buffer
    unsigned int m_height;  //!< Height of the P-Buffer
};

} // namespace priv

} // namespace sf


#endif // SFML_RENDERTEXTUREIMPLDEFAULT_HPP

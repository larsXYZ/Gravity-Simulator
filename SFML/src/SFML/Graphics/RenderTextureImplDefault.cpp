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
#include <SFML/Graphics/RenderTextureImplDefault.hpp>
#include <SFML/Graphics/GLCheck.hpp>
#include <SFML/Graphics/TextureSaver.hpp>
#include <SFML/Window/Context.hpp>


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
RenderTextureImplDefault::RenderTextureImplDefault() :
m_context(0),
m_width  (0),
m_height (0)
{

}


////////////////////////////////////////////////////////////
RenderTextureImplDefault::~RenderTextureImplDefault()
{
    // Destroy the context
    delete m_context;
}


////////////////////////////////////////////////////////////
unsigned int RenderTextureImplDefault::getMaximumAntialiasingLevel()
{
    // If the system is so old that it doesn't support FBOs, chances are it is
    // also using either a software renderer or some CPU emulated support for AA
    // In order to not cripple performance in this rare case, we just return 0 here
    return 0;
}


////////////////////////////////////////////////////////////
bool RenderTextureImplDefault::create(unsigned int width, unsigned int height, unsigned int, const ContextSettings& settings)
{
    // Store the dimensions
    m_width = width;
    m_height = height;

    // Create the in-memory OpenGL context
    m_context = new Context(settings, width, height);

    return true;
}


////////////////////////////////////////////////////////////
bool RenderTextureImplDefault::activate(bool active)
{
    return m_context->setActive(active);
}


////////////////////////////////////////////////////////////
bool RenderTextureImplDefault::isSrgb() const
{
    return m_context->getSettings().sRgbCapable;
}


////////////////////////////////////////////////////////////
void RenderTextureImplDefault::updateTexture(unsigned int textureId)
{
    // Make sure that the current texture binding will be preserved
    priv::TextureSaver save;

    // Copy the rendered pixels to the texture
    glCheck(glBindTexture(GL_TEXTURE_2D, textureId));
    glCheck(glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height)));
}

} // namespace priv

} // namespace sf

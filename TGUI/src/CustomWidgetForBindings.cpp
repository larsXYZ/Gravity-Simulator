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

#include <TGUI/CustomWidgetForBindings.hpp>

#ifndef TGUI_REMOVE_DEPRECATED_CODE

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
#if TGUI_COMPILED_WITH_CPP_VER < 17
    constexpr const char CustomWidgetForBindings::StaticWidgetType[];
#endif

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    CustomWidgetForBindings::CustomWidgetForBindings(const char* typeName, bool initRenderer) :
        Widget{typeName, initRenderer}
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    CustomWidgetForBindings::Ptr CustomWidgetForBindings::create()
    {
        return std::make_shared<CustomWidgetForBindings>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::setPosition(const Layout2d& position)
    {
        Widget::setPosition(position);
        implPositionChanged(getPosition());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::setSize(const Layout2d& size)
    {
        Widget::setSize(size);
        implSizeChanged(getSize());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Vector2f CustomWidgetForBindings::getFullSize() const
    {
        return implGetFullSize();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Vector2f CustomWidgetForBindings::getWidgetOffset() const
    {
        return implGetWidgetOffset();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::setVisible(bool visible)
    {
        Widget::setVisible(visible);
        implVisibleChanged(visible);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::setEnabled(bool enabled)
    {
        Widget::setEnabled(enabled);
        implEnableChanged(enabled);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::setFocused(bool focused)
    {
        Widget::setFocused(focused);
        implFocusChanged(m_focused);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool CustomWidgetForBindings::canGainFocus() const
    {
        return implCanGainFocus();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool CustomWidgetForBindings::updateTime(Duration elapsedTime)
    {
        bool screenRefreshRequired = Widget::updateTime(elapsedTime);
        screenRefreshRequired |= implUpdateTimeFunction(elapsedTime);
        return screenRefreshRequired;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool CustomWidgetForBindings::isMouseOnWidget(Vector2f pos) const
    {
        return implMouseOnWidget(pos);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool CustomWidgetForBindings::leftMousePressed(Vector2f pos)
    {
        Widget::leftMousePressed(pos);
        return implLeftMousePressed(pos);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::leftMouseReleased(Vector2f pos)
    {
        Widget::leftMouseReleased(pos);
        implLeftMouseReleased(pos);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::rightMousePressed(Vector2f pos)
    {
        Widget::rightMousePressed(pos);
        implRightMousePressed(pos);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::rightMouseReleased(Vector2f pos)
    {
        Widget::rightMouseReleased(pos);
        implRightMouseReleased(pos);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::mouseMoved(Vector2f pos)
    {
        Widget::mouseMoved(pos);
        implMouseMoved(pos);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::keyPressed(const Event::KeyEvent& event)
    {
        Widget::keyPressed(event);
        implKeyPressed(event);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::textEntered(char32_t key)
    {
        Widget::textEntered(key);
        implTextEntered(key);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool CustomWidgetForBindings::scrolled(float delta, Vector2f pos, bool touch)
    {
        return implScrolled(delta, pos, touch);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::mouseNoLongerOnWidget()
    {
        Widget::mouseNoLongerOnWidget();
        implMouseNoLongerOnWidget();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::leftMouseButtonNoLongerDown()
    {
        Widget::leftMouseButtonNoLongerDown();
        implLeftMouseButtonNoLongerDown();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::rendererChanged(const String& property)
    {
        if (!implRendererChanged(property))
            Widget::rendererChanged(property);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::mouseEnteredWidget()
    {
        Widget::mouseEnteredWidget();
        implMouseEnteredWidget();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::mouseLeftWidget()
    {
        Widget::mouseLeftWidget();
        implMouseLeftWidget();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CustomWidgetForBindings::draw(BackendRenderTarget& target, RenderStates states) const
    {
        implDrawFunction(target, states);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Widget::Ptr CustomWidgetForBindings::clone() const
    {
        return std::make_shared<CustomWidgetForBindings>(*this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif // TGUI_REMOVE_DEPRECATED_CODE

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

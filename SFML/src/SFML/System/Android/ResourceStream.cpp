////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2013 Jonathan De Wachter (dewachter.jonathan@gmail.com)
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
#include <SFML/System/Android/ResourceStream.hpp>
#include <SFML/System/Android/Activity.hpp>
#include <SFML/System/Lock.hpp>


namespace sf
{
namespace priv
{

////////////////////////////////////////////////////////////
ResourceStream::ResourceStream(const std::string& filename) :
m_file (NULL)
{
    ActivityStates& states = getActivity();
    Lock lock(states.mutex);
    m_file = AAssetManager_open(states.activity->assetManager, filename.c_str(), AASSET_MODE_UNKNOWN);
}


////////////////////////////////////////////////////////////
ResourceStream::~ResourceStream()
{
    if (m_file)
    {
        AAsset_close(m_file);
    }
}


////////////////////////////////////////////////////////////
Int64 ResourceStream::read(void *data, Int64 size)
{
    if (m_file)
    {
        return AAsset_read(m_file, data, static_cast<size_t>(size));
    }
    else
    {
        return -1;
    }
}


////////////////////////////////////////////////////////////
Int64 ResourceStream::seek(Int64 position)
{
    if (m_file)
    {
        return AAsset_seek(m_file, static_cast<off_t>(position), SEEK_SET);
    }
    else
    {
        return -1;
    }
}


////////////////////////////////////////////////////////////
Int64 ResourceStream::tell()
{
    if (m_file)
    {
        return getSize() - AAsset_getRemainingLength(m_file);
    }
    else
    {
        return -1;
    }
}


////////////////////////////////////////////////////////////
Int64 ResourceStream::getSize()
{
    if (m_file)
    {
        return AAsset_getLength(m_file);
    }
    else
    {
        return -1;
    }
}


} // namespace priv
} // namespace sf

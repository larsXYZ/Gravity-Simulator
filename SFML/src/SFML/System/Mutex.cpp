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
#include <SFML/System/Mutex.hpp>

#if defined(SFML_SYSTEM_WINDOWS)
    #include <SFML/System/Win32/MutexImpl.hpp>
#else
    #include <SFML/System/Unix/MutexImpl.hpp>
#endif


namespace sf
{
////////////////////////////////////////////////////////////
Mutex::Mutex()
{
    m_mutexImpl = new priv::MutexImpl;
}


////////////////////////////////////////////////////////////
Mutex::~Mutex()
{
    delete m_mutexImpl;
}


////////////////////////////////////////////////////////////
void Mutex::lock()
{
    m_mutexImpl->lock();
}


////////////////////////////////////////////////////////////
void Mutex::unlock()
{
    m_mutexImpl->unlock();
}

} // namespace sf

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

#ifndef SFML_SOUNDFILEWRITEROGG_HPP
#define SFML_SOUNDFILEWRITEROGG_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/SoundFileWriter.hpp>
#include <vorbis/vorbisenc.h>
#include <fstream>


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
/// \brief Implementation of sound file writer that handles OGG/Vorbis files
///
////////////////////////////////////////////////////////////
class SoundFileWriterOgg : public SoundFileWriter
{
public:

    ////////////////////////////////////////////////////////////
    /// \brief Check if this writer can handle a file on disk
    ///
    /// \param filename Path of the sound file to check
    ///
    /// \return True if the file can be written by this writer
    ///
    ////////////////////////////////////////////////////////////
    static bool check(const std::string& filename);

public:

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    SoundFileWriterOgg();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~SoundFileWriterOgg();

    ////////////////////////////////////////////////////////////
    /// \brief Open a sound file for writing
    ///
    /// \param filename     Path of the file to open
    /// \param sampleRate   Sample rate of the sound
    /// \param channelCount Number of channels of the sound
    ///
    /// \return True if the file was successfully opened
    ///
    ////////////////////////////////////////////////////////////
    virtual bool open(const std::string& filename, unsigned int sampleRate, unsigned int channelCount);

    ////////////////////////////////////////////////////////////
    /// \brief Write audio samples to the open file
    ///
    /// \param samples Pointer to the sample array to write
    /// \param count   Number of samples to write
    ///
    ////////////////////////////////////////////////////////////
    virtual void write(const Int16* samples, Uint64 count);

private:

    ////////////////////////////////////////////////////////////
    /// \brief Flush blocks produced by the ogg stream, if any
    ///
    ////////////////////////////////////////////////////////////
    void flushBlocks();

    ////////////////////////////////////////////////////////////
    /// \brief Close the file
    ///
    ////////////////////////////////////////////////////////////
    void close();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    unsigned int     m_channelCount; // channel count of the sound being written
    std::ofstream    m_file;         // output file
    ogg_stream_state m_ogg;          // ogg stream
    vorbis_info      m_vorbis;       // vorbis handle
    vorbis_dsp_state m_state;        // current encoding state
};

} // namespace priv

} // namespace sf


#endif // SFML_SOUNDFILEWRITEROGG_HPP

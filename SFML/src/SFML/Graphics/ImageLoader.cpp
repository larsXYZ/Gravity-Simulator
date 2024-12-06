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
#include <SFML/Graphics/ImageLoader.hpp>
#include <SFML/System/InputStream.hpp>
#include <SFML/System/Err.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <cctype>
#include <iterator>


namespace
{
    // Convert a string to lower case
    std::string toLower(std::string str)
    {
        for (std::string::iterator i = str.begin(); i != str.end(); ++i)
            *i = static_cast<char>(std::tolower(*i));
        return str;
    }

    // stb_image callbacks that operate on a sf::InputStream
    int read(void* user, char* data, int size)
    {
        sf::InputStream* stream = static_cast<sf::InputStream*>(user);
        return static_cast<int>(stream->read(data, size));
    }
    void skip(void* user, int size)
    {
        sf::InputStream* stream = static_cast<sf::InputStream*>(user);
        stream->seek(stream->tell() + size);
    }
    int eof(void* user)
    {
        sf::InputStream* stream = static_cast<sf::InputStream*>(user);
        return stream->tell() >= stream->getSize();
    }

    // stb_image callback for constructing a buffer
    void bufferFromCallback(void* context, void* data, int size)
    {
        sf::Uint8* source = static_cast<sf::Uint8*>(data);
        std::vector<sf::Uint8>* dest = static_cast<std::vector<sf::Uint8>*>(context);
        std::copy(source, source + size, std::back_inserter(*dest));
    }
}


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
ImageLoader& ImageLoader::getInstance()
{
    static ImageLoader Instance;

    return Instance;
}


////////////////////////////////////////////////////////////
ImageLoader::ImageLoader()
{
    // Nothing to do
}


////////////////////////////////////////////////////////////
ImageLoader::~ImageLoader()
{
    // Nothing to do
}


////////////////////////////////////////////////////////////
bool ImageLoader::loadImageFromFile(const std::string& filename, std::vector<Uint8>& pixels, Vector2u& size)
{
    // Clear the array (just in case)
    pixels.clear();

    // Load the image and get a pointer to the pixels in memory
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* ptr = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (ptr)
    {
        // Assign the image properties
        size.x = static_cast<unsigned int>(width);
        size.y = static_cast<unsigned int>(height);

        if (width > 0 && height > 0)
        {
            // Copy the loaded pixels to the pixel buffer
            pixels.resize(static_cast<std::size_t>(width * height * 4));
            memcpy(&pixels[0], ptr, pixels.size());
        }

        // Free the loaded pixels (they are now in our own pixel buffer)
        stbi_image_free(ptr);

        return true;
    }
    else
    {
        // Error, failed to load the image
        err() << "Failed to load image \"" << filename << "\". Reason: " << stbi_failure_reason() << std::endl;

        return false;
    }
}


////////////////////////////////////////////////////////////
bool ImageLoader::loadImageFromMemory(const void* data, std::size_t dataSize, std::vector<Uint8>& pixels, Vector2u& size)
{
    // Check input parameters
    if (data && dataSize)
    {
        // Clear the array (just in case)
        pixels.clear();

        // Load the image and get a pointer to the pixels in memory
        int width = 0;
        int height = 0;
        int channels = 0;
        const unsigned char* buffer = static_cast<const unsigned char*>(data);
        unsigned char* ptr = stbi_load_from_memory(buffer, static_cast<int>(dataSize), &width, &height, &channels, STBI_rgb_alpha);

        if (ptr)
        {
            // Assign the image properties
            size.x = static_cast<unsigned int>(width);
            size.y = static_cast<unsigned int>(height);

            if (width > 0 && height > 0)
            {
                // Copy the loaded pixels to the pixel buffer
                pixels.resize(static_cast<std::size_t>(width * height * 4));
                memcpy(&pixels[0], ptr, pixels.size());
            }

            // Free the loaded pixels (they are now in our own pixel buffer)
            stbi_image_free(ptr);

            return true;
        }
        else
        {
            // Error, failed to load the image
            err() << "Failed to load image from memory. Reason: " << stbi_failure_reason() << std::endl;

            return false;
        }
    }
    else
    {
        err() << "Failed to load image from memory, no data provided" << std::endl;
        return false;
    }
}


////////////////////////////////////////////////////////////
bool ImageLoader::loadImageFromStream(InputStream& stream, std::vector<Uint8>& pixels, Vector2u& size)
{
    // Clear the array (just in case)
    pixels.clear();

    // Make sure that the stream's reading position is at the beginning
    stream.seek(0);

    // Setup the stb_image callbacks
    stbi_io_callbacks callbacks;
    callbacks.read = &read;
    callbacks.skip = &skip;
    callbacks.eof  = &eof;

    // Load the image and get a pointer to the pixels in memory
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* ptr = stbi_load_from_callbacks(&callbacks, &stream, &width, &height, &channels, STBI_rgb_alpha);

    if (ptr)
    {
        // Assign the image properties
        size.x = static_cast<unsigned int>(width);
        size.y = static_cast<unsigned int>(height);

        if (width && height)
        {
            // Copy the loaded pixels to the pixel buffer
            pixels.resize(static_cast<std::size_t>(width * height * 4));
            memcpy(&pixels[0], ptr, pixels.size());
        }

        // Free the loaded pixels (they are now in our own pixel buffer)
        stbi_image_free(ptr);

        return true;
    }
    else
    {
        // Error, failed to load the image
        err() << "Failed to load image from stream. Reason: " << stbi_failure_reason() << std::endl;

        return false;
    }
}


////////////////////////////////////////////////////////////
bool ImageLoader::saveImageToFile(const std::string& filename, const std::vector<Uint8>& pixels, const Vector2u& size)
{
    // Make sure the image is not empty
    if (!pixels.empty() && (size.x > 0) && (size.y > 0))
    {
        // Deduce the image type from its extension

        // Extract the extension
        const std::size_t dot = filename.find_last_of('.');
        const std::string extension = dot != std::string::npos ? toLower(filename.substr(dot + 1)) : "";
        const Vector2i convertedSize = Vector2i(size);

        if (extension == "bmp")
        {
            // BMP format
            if (stbi_write_bmp(filename.c_str(), convertedSize.x, convertedSize.y, 4, &pixels[0]))
                return true;
        }
        else if (extension == "tga")
        {
            // TGA format
            if (stbi_write_tga(filename.c_str(), convertedSize.x, convertedSize.y, 4, &pixels[0]))
                return true;
        }
        else if (extension == "png")
        {
            // PNG format
            if (stbi_write_png(filename.c_str(), convertedSize.x, convertedSize.y, 4, &pixels[0], 0))
                return true;
        }
        else if (extension == "jpg" || extension == "jpeg")
        {
            // JPG format
            if (stbi_write_jpg(filename.c_str(), convertedSize.x, convertedSize.y, 4, &pixels[0], 90))
                return true;
        }
    }

    err() << "Failed to save image \"" << filename << "\"" << std::endl;
    return false;
}

////////////////////////////////////////////////////////////
bool ImageLoader::saveImageToMemory(const std::string& format, std::vector<sf::Uint8>& output, const std::vector<Uint8>& pixels, const Vector2u& size)
{
    // Make sure the image is not empty
    if (!pixels.empty() && (size.x > 0) && (size.y > 0))
    {
        // Choose function based on format

        std::string specified = toLower(format);
        const Vector2i convertedSize = Vector2i(size);

        if (specified == "bmp")
        {
            // BMP format
            if (stbi_write_bmp_to_func(&bufferFromCallback, &output, convertedSize.x, convertedSize.y, 4, &pixels[0]))
                return true;
        }
        else if (specified == "tga")
        {
            // TGA format
            if (stbi_write_tga_to_func(&bufferFromCallback, &output, convertedSize.x, convertedSize.y, 4, &pixels[0]))
                return true;
        }
        else if (specified == "png")
        {
            // PNG format
            if (stbi_write_png_to_func(&bufferFromCallback, &output, convertedSize.x, convertedSize.y, 4, &pixels[0], 0))
                return true;
        }
        else if (specified == "jpg" || specified == "jpeg")
        {
            // JPG format
            if (stbi_write_jpg_to_func(&bufferFromCallback, &output, convertedSize.x, convertedSize.y, 4, &pixels[0], 90))
                return true;
        }
    }

    err() << "Failed to save image with format \"" << format << "\"" << std::endl;
    return false;
}

} // namespace priv

} // namespace sf

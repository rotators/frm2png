/*
 * Copyright (c) 2015-2018 Falltergeist developers
 * Copyright (c) 2019 Rotators
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

// C++ standard includes
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>

// frm2png includes
#include "PngImage.h"
#include "PngWriter.h"

// Third party includes
#include <png.h>

namespace frm2png
{
    PngWriter::PngWriter( const std::string& filename )
    {
        _stream.open( filename, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
        if( !_stream.is_open() )
            throw std::runtime_error( "PngWriter::PngWriter() - Can't open output file:" + filename );

        // Initialize write structure
        _png_write = png_create_write_struct( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr );
        if( _png_write == nullptr )
            throw std::runtime_error( "PngWriter::PngWriter() - Could not allocate write struct" );

        // Initialize info structure
        _png_info = png_create_info_struct( _png_write );
        if( _png_info == nullptr )
            throw std::runtime_error( "PngWriter::PngWriter() - Could not allocate info struct" );

        // Setup Exception handling
        if( setjmp( png_jmpbuf( _png_write ) ) )
            throw std::runtime_error( "PngWriter::PngWriter() - Error during png creation" );

        png_set_write_fn( _png_write, &_stream, PngWriter::writeCallback, PngWriter::flushCallback );
    }

    PngWriter::~PngWriter()
    {
        png_destroy_write_struct( &_png_write, &_png_info );
        _stream.close();
    }

    void PngWriter::writeCallback( png_structp png_write, png_bytep data, png_size_t length )
    {
        std::ofstream* stream = (std::ofstream*)png_get_io_ptr( png_write );
        stream->write( (char*)data, length );
    }

    void PngWriter::flushCallback( png_structp png_ptr )
    {
        std::ofstream* stream = (std::ofstream*)png_get_io_ptr( png_ptr ); //Get pointer to ostream
        stream->flush();
    }

    void PngWriter::write( const PngImage& image )
    {
        // IHDR chunk
        png_set_IHDR( _png_write, _png_info,
                      image.width(), image.height(),
                      8,
                      PNG_COLOR_TYPE_RGB_ALPHA,
                      PNG_INTERLACE_NONE,
                      PNG_COMPRESSION_TYPE_DEFAULT,
                      PNG_FILTER_TYPE_DEFAULT );
        png_write_info( _png_write, _png_info );

        // IDAT chunk
        png_write_image( _png_write, image.rows() );

        // IEND chunk
        png_write_end( _png_write, _png_info );
    }

    void PngWriter::writeAnimHeader( uint32_t width, uint32_t height, uint32_t frames, uint32_t loop, bool preview )
    {
        // IHDR chunk
        png_set_IHDR( _png_write, _png_info,
                      width, height,
                      8,
                      PNG_COLOR_TYPE_RGB_ALPHA,
                      PNG_INTERLACE_NONE,
                      PNG_COMPRESSION_TYPE_DEFAULT,
                      PNG_FILTER_TYPE_DEFAULT );

        // acTL chunk
        // if( frames < 2 )
        //     throw std::runtime_error( "PngWriter::writeAnimHeader() - Invalid number of frames (" + std::to_string( frames ) + " < 2)" );

        png_set_acTL( _png_write, _png_info, frames, loop );
        png_write_info( _png_write, _png_info );

        png_set_first_frame_is_hidden( _png_write, _png_info, preview );
    }

    void PngWriter::writeAnimFrame( const PngImage& image, uint32_t offsetX, uint32_t offsetY, uint16_t delayNum, uint16_t delayDen, uint8_t dispose, uint8_t blend )
    {
        // fcTL chunk
        png_write_frame_head( _png_write, _png_info,
                              nullptr,                       // rows (unused)
                              image.width(), image.height(), // first frame must match IHDR
                              offsetX, offsetY,              // first frame must use 0
                              delayNum, delayDen,
                              dispose, blend );

        // IDAT/fdAT chunk
        png_write_image( _png_write, image.rows() );
        png_write_frame_tail( _png_write, _png_info );
    }

    void PngWriter::writeAnimEnd()
    {
        // IEND chunk
        png_write_end( _png_write, _png_info );
    }
}

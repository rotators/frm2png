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
#include <string>

// frm2png includes
#include "Exception.h"
#include "PngImage.h"

// Third party includes

namespace frm2png
{
    PngImage::PngImage( uint32_t width, uint32_t height )
    {
        _width = width;
        _height = height;

        _rows = new png_bytep[_height]();

        for( uint32_t y = 0; y != _height; ++y)
        {
            _rows[y] = new png_byte[_width*4]();
        }
    }

    PngImage::~PngImage()
    {
        for( uint32_t y = 0; y != _height; ++y )
        {
            delete [] _rows[y];
        }
        delete [] _rows;
    }

    void PngImage::setPixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha /* = 255 */)
    {
        if( x > _width || y > _height )
            throw Exception( "PngImage::setPixel() - Invalid position " + std::to_string(x) + "," + std::to_string(y) );

        _rows[y][x*4] = r;
        _rows[y][x*4 + 1] = g;
        _rows[y][x*4 + 2] = b;
        _rows[y][x*4 + 3] = alpha;
    }

    uint32_t PngImage::width() const
    {
        return _width;
    }

    uint32_t PngImage::height() const
    {
        return _height;
    }

    png_bytepp PngImage::rows() const
    {
        return _rows;
    }
}

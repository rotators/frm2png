/*
 * Copyright (c) 2015-2018 Falltergeist developers
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

#pragma once

// C++ standard includes
#include <cstdint>
#include <fstream>
#include <string>

// frm2png includes
#include "PngImage.h"

// Third party includes
#include <png.h>

namespace frm2png
{
    class PngWriter
    {
        protected:
            std::ofstream _stream;
            png_structp _png_write;
            png_infop _png_info;

        public:
            PngWriter(const std::string& filename);
            ~PngWriter();

        protected:
            static void writeCallback(png_structp png_struct, png_bytep data, png_size_t length);
            static void flushCallback(png_structp png_ptr);

        public:
            void write(const PngImage& image);

            void writeAnimHeader( uint32_t width, uint32_t height, uint32_t frames, uint32_t loop, bool preview );
            void writeAnimFrame( const PngImage& image, uint32_t offsetX, uint32_t offsetY, uint16_t delayNum, uint16_t delayDen, uint8_t dispose, uint8_t blend );
            void writeAnimEnd();
    };
}

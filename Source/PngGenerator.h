/*
 * Copyright (c) 2019-2021 Rotators
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

// c++ includes
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

// frm2png includes
#include "Logging.h"

// falltergeist includes
#include "Format/Frm/File.h"
#include "Format/Pal/File.h"

namespace frm2png
{
    struct PngGeneratorData
    {
        Falltergeist::Format::Frm::File Frm;
        Falltergeist::Format::Pal::File Pal;

        uint8_t RgbMultiplier = 0;

        std::string PngPath;
        std::string PngBasename;
        std::string PngExtension;

        PngGeneratorData( Falltergeist::Format::Frm::File&& frm, Falltergeist::Format::Pal::File&& pal );
    };

    typedef std::function<void( const PngGeneratorData&, Logging& )> PngGeneratorFunc;

    extern std::unordered_map<std::string, PngGeneratorFunc> Generator;

    void InitPngGenerators();
}

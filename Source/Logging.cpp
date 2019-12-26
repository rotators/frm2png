/*
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

// c++ includes
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// frm2png includes
#include "Logging.h"

namespace frm2png
{
    Logging::Logging( bool enabled /* = false */, bool cache /* = false */, size_t indent /* = 0 */ ) :
        Enabled( enabled ),
        Cache( cache ),
        Indent( indent )
    {}

    void Logging::Clear()
    {
        Enabled = Cache = false;
        Indent          = 0;
        Cached.clear();
    }

    Logging& Logging::operator<<( const int8_t& indent )
    {
        if( Enabled )
            Indent += indent;

        return *this;
    }

    Logging& Logging::operator<<( const std::string& message )
    {
        if( Enabled )
        {
            if( Cache )
                Cached.push_back( std::make_pair( Indent, message ) );
            else
                std::cout << "> " << std::string( Indent, ' ' ) << message << std::endl;
        }

        return *this;
    }
}

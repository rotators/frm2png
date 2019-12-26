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
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

// frm2png includes
#include "ColorPal.h"
#include "PngImage.h"
#include "PngWriter.h"

// falltergeist includes
#include "Format/Dat/Stream.h"
#include "Format/Frm/File.h"

// third party includes
#include <clipp.h>

using namespace frm2png;

struct Options
{
    clipp::group CommandLine;

    bool Help = false;
    bool Version = false;

    // input
    std::string DatFile;
    std::string PalFile;
    std::string PalName = "default";
    std::string FrmFile;

    // output
    std::string Generator;
    std::string PngFile;

    // misc
    bool Verbose = false;

    Options()
    {}

    void Init()
    {
        auto cmdInfo = (
            clipp::option( "--help", "-h" ).set( Help ).doc( "show help summary" ) |
            clipp::option( "--version", "-v" ).set( Version ).doc( "show program version" )
        ).doc( "General options\n" );

        auto cmdInput = (
            (clipp::option( "-d", "--dat" ) & clipp::value("DAT", DatFile )).doc( "Use specified DAT file" ),
            (clipp::option( "-p", "--pal" ) & clipp::value( "PAL", PalFile )).doc( "Use specified PAL file" ) |
            (clipp::option( "-P", "--palette" ) & clipp::value( "name", PalName )).doc( "Use embedded palette" )
        ).doc( "Input options" );

        auto cmdOutput = (
            (clipp::option( "-g", "--generator" ) & clipp::word( "name", Generator )).doc( "generator" ),
            (clipp::option( "-o", "--output" ) & clipp::value( "PNG", PngFile )).doc( "output filename" )
        ).doc( "Output options" );

        auto cmdMisc = (
            clipp::option( "-V", "--verbose").set( Verbose ).doc( "prints various debug messages" ),
            clipp::option( "-X", "--clean" )
        ).doc( "Misc options" );

        CommandLine = (cmdInfo | (cmdInput, cmdOutput, cmdMisc, clipp::value( "filename.frm", FrmFile )));
    }
};

struct Logging
{
    bool        Enabled;
    bool        Cache;
    std::size_t Indent;

    std::vector<std::pair<std::size_t, std::string>> Cached;

    Logging() :
        Enabled( false ),
        Cache( false ),
        Indent( 0 )
    {}

    void Clear()
    {
        Enabled = Cache = false;
        Indent = 0;
        Cached.clear();
    }

    Logging& operator<<( const int8_t& indent )
    {
        if( Enabled )
            Indent += indent;

        return *this;
    }

    Logging& operator<<( const std::string& message )
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
};

void printVersion()
{
    std::cout << "FRM to PNG converter v0.1.5r" << std::endl
              << "Copyright (c) 2015-2018 Falltergeist developers" << std::endl
              << "Copyright (c) 2019 Rotators" << std::endl
              << std::endl;
}

int exitVersion()
{
    printVersion();

    return EXIT_SUCCESS;
}

int exitHelp( int exitCode, Options& options )
{
    printVersion();

    auto cmdFormat = clipp::doc_formatting{}
                     .first_column( 2 )
                     .indent_size( 2 )
                     .doc_column( 30 )
                     .last_column( 78 );

    std::cout << "Usage:" << std::endl
              << clipp::usage_lines( options.CommandLine, "frm2png", cmdFormat ) << std::endl
              << std::endl;

    cmdFormat.first_column( 0 );

    std::cout << clipp::documentation( options.CommandLine, cmdFormat ) << std::endl
              << std::endl;

    return exitCode;
}

void printFRM(const std::string& filename, Falltergeist::Format::Frm::File& frm)
{
    std::cout << "=== FRM info ===" << std::endl;
    std::cout << "Filename ..............  " << filename << std::endl;
    std::cout << "Version ................ " << frm.version() << std::endl;
    std::cout << "Frames per second ...... " << frm.framesPerSecond() << std::endl;
    std::cout << "Action frame ........... " << frm.actionFrame() << std::endl;
    std::cout << "Directions ............. " << frm.directions().size() << std::endl;
    std::cout << "Frames per direction ... " << frm.framesPerDirection() << std::endl;
}

// <- path/to/file.ext
// -> path/to/
// -> file
// -> .ext
void splitFilename( const std::string& full, std::string& path, std::string& basename, std::string& extension )
{
    size_t pos;
    std::string filename;

    pos = full.find_last_of( '/' );
    if( pos != std::string::npos )
    {
        path = full.substr( 0, pos + 1 );
        filename = full.substr( pos + 1 );

        if( path == "." )
            path.clear();
    }
    else
    {
        path = "";
        filename = full;
    }

    pos = filename.find_last_of( '.' );
    if( pos != std::string::npos )
    {
        basename = filename.substr( 0, pos );
        extension = filename.substr( pos );
    }
    else
    {
        basename = filename;
        extension = "";
    }
}

template<typename T>
T loadFile(const std::string& filename)
{
    std::ifstream stream;

    stream.open(filename, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open())
        throw std::runtime_error( "openStreamFile() - Can't open input file: " + filename );

    T obj = T( stream );

    return obj;
}

Falltergeist::Format::Pal::File loadPal( const Options& options )
{
    if( !options.PalFile.empty() )
        return loadFile<Falltergeist::Format::Pal::File>( options.PalFile );

    std::string palName = options.PalName;
    if( palName.empty() )
        palName = "default";

    auto it = ColorPal.find( palName );
    if( it != ColorPal.end() )
    {
        Falltergeist::Format::Pal::File pal( it->second );

        return pal;
    }

    throw std::runtime_error( "loadPal() - unknown palette name '" + palName + "'" );
}

int main(int argc, char** argv)
{
   Logging verbose;
   Options options;
   options.Init();

    auto optionsResult = clipp::parse( argc, argv, options.CommandLine );

    verbose.Enabled = options.Verbose;
    verbose << "command line" << 1
            << "Help      = " + std::string(options.Help ? "true" : "false")
            << "Version   = " + std::string(options.Version ? "true" : "false")
            // input
            << "DatFile   = " + options.DatFile
            << "PalFile   = " + options.PalFile
            << "PalName   = " + options.PalName
            << "FrmFile   = " + options.FrmFile
            // output
            << "Generator = " + options.Generator
            << "PngFile   = " + options.PngFile
            // misc
            << "Verbose   = " + std::string(options.Verbose ? "true" : "false" )
            << -1;

    if( !optionsResult )
        return exitHelp( EXIT_FAILURE, options );
    else if( options.Help )
        return exitHelp( EXIT_SUCCESS, options );
    else if( options.Version )
        return exitVersion();

    // split output filename into few parts; helps generators to modify filename provided by user

    std::string pngPath, pngBasename, pngExtension, pngFull;

    if( !options.PngFile.empty() )
        pngFull = options.PngFile;
    else
    {
        std::string frmPath, frmBasename, frmExtension;

        splitFilename( options.FrmFile, frmPath, frmBasename, frmExtension );
        pngFull = frmPath + frmBasename + ".png";
    }

    splitFilename( pngFull, pngPath, pngBasename, pngExtension );

    verbose << "png = full[" + pngFull + "] path[" + pngPath + "] basename[" + pngBasename + "] extension[" + pngExtension + "]";

    verbose << "enter main block" << 1;
    try
    {
        verbose << "load frm";
        auto frm = loadFile<Falltergeist::Format::Frm::File>( options.FrmFile );
        printFRM( options.FrmFile, frm );

        verbose << "load palette";
        Falltergeist::Format::Pal::File pal = loadPal( options );

        // TODO? make rgbMultiplier configurable
        static constexpr uint8_t rgbMultiplier = 4;

        //if (apng && frm.directions().size() == 1)
        //    apng = false;

        verbose << "select generator";
        if ( options.Generator.empty() )
        {
            verbose << "start generator = legacy" << 1;

            // draw .png

            uint16_t maxWidth = frm.maxFrameWidth();
            uint16_t maxHeight = frm.maxFrameHeight();

            PngImage image( maxWidth * frm.framesPerDirection(), maxHeight * static_cast<uint8_t>( frm.directions().size() ));

            uint8_t  dirIdx = 0;
            for( const auto& dir : frm.directions() )
            {
                uint16_t frameIdx = 0;

                for( const auto& frame : dir.frames() )
                {
                    const uint32_t pngX = maxWidth * frameIdx;
                    const uint32_t pngY = maxHeight * dirIdx;

                    verbose << "draw dir:" + std::to_string(dirIdx) + " frame:" + std::to_string(frameIdx)
                            + " @ " + std::to_string(pngX) + "," + std::to_string(pngY)
                            + " -> " + std::to_string(frame.width()) + "x" + std::to_string(frame.height())
                            ;

                    for (uint16_t y = 0, h = frame.height(); y < h; y++) {
                        for (uint16_t x = 0, w = frame.width(); x < w; x++) {
                            const Falltergeist::Format::Pal::Color* color = pal.color(frame.index(x, y));
                            image.setPixel(pngX + x, pngY + y, color->red() * rgbMultiplier, color->green()  * rgbMultiplier, color->blue()  * rgbMultiplier, color->alpha());
                        }
                    }
                    frameIdx++;
                }
                dirIdx++;
            }

            verbose << -1 << "end generator = legacy";

            verbose << "write png = " + pngFull + " = " + std::to_string( image.width() ) + "x" + std::to_string( image.height() );
            PngWriter png( pngFull );
            png.write( image );
        }
        else if( options.Generator == "apng" ) // experimental apng support
        {
            verbose << "start generator = apng" << 1;

            uint8_t dirIdx = 0;
            for (const auto& dir : frm.directions()) {
                // TODO support acTL + fcTL + IDAT variant
                static constexpr bool firstIsAnim = false;

                // .frm frames iteration [1/3]; init conversion from .frm offsets to .png offsets

                uint32_t pngWidth = 0, pngHeight = 0;
                int32_t spotX = 0, spotY = 0;
                std::vector<std::pair<int32_t,int32_t>> offset;

                uint16_t frameIdx = 0;
                for (const auto& frame : dir.frames()) {
                    if (!frameIdx) {
                        // save first .frm frame spot position

                        spotX = frame.width() / 2;
                        spotY = frame.height();

                        offset.emplace_back( 0, 0 );
                    } else {
                        // apply .frm offsets to cached spot

                        spotX += frame.offsetX();
                        spotY += frame.offsetY();

                        // convert .frm spot to .png offset

                        offset.emplace_back( spotX - frame.width() / 2, spotY - frame.height() );
                        verbose << "offset dir:" + std::to_string(dirIdx) + " frame:" + std::to_string(frameIdx) + " = " + std::to_string(offset.back().first) + "," + std::to_string(offset.back().second);

                        // shift position of first frame, if needed

                        if (offset.back().first < 0)
                            offset.front().first = std::max(offset.front().first, -offset.back().first);

                        if (offset.back().second < 0)
                            offset.front().second = std::max(offset.front().second, -offset.back().second);
                    }
                    frameIdx++;
                }

                // .frm frames iteration [2/3]; finish conversion from .frm offsets to .png offsets, find .png image size

                frameIdx = 0;
                for (auto& frameOffset : offset) {
                    // apply first frame offset to other frames

                    if (frameIdx) {
                        frameOffset.first += offset.front().first;
                        frameOffset.second += offset.front().second;
                    }

                    pngWidth = std::max<uint32_t>(pngWidth, frameOffset.first + dir.frames().at(frameIdx).width());
                    pngHeight = std::max<uint32_t>(pngHeight, frameOffset.second + dir.frames().at(frameIdx).height());

                    frameIdx++;
                }

                const std::string pngName = pngPath + pngBasename + "_" + std::to_string( dirIdx ) + pngExtension;

                verbose << "write png = " + pngName + " = " + std::to_string(pngWidth) + "x" + std::to_string(pngHeight) << 1;
                PngWriter png( pngName );

                png.writeAnimHeader( pngWidth, pngHeight, static_cast<uint16_t>( dir.frames().size() ) + (firstIsAnim ? 0 : 1), 0, !firstIsAnim );

                if (!firstIsAnim) {
                    // if first image is not supposed to be part of animation, copy of first frame is added and moved to center
                    // software supporting APNG will ignore it, anything else will use that as image to display

                    auto& frame = dir.frames().front();
                    PngImage defaultImage(pngWidth, pngHeight);

                    // draw .png frame

                    const uint32_t pngX = pngWidth / 2 - frame.width() / 2;
                    const uint32_t pngY = pngHeight / 2 - frame.height() / 2;
                    for (uint16_t x = 0, w = frame.width(); x < w; x++) {
                        for (uint16_t y = 0, h = frame.height(); y < h; y++) {
                            const Falltergeist::Format::Pal::Color* color = pal.color(frame.index(x, y));
                            defaultImage.setPixel(pngX + x, pngY + y, color->red() * rgbMultiplier, color->green() * rgbMultiplier, color->blue() * rgbMultiplier, color->alpha());
                        }
                    }

                    png.writeAnimFrame( defaultImage, 0, 0, 0, 0, 0, 0 );
                }

                // .frm frames iteration [3/3]; draw .png frames
                frameIdx = 0;
                for (const auto& frame : dir.frames()) {

                    // draw .png frame

                    verbose << "draw frame:" + std::to_string(frameIdx) + " @ " + std::to_string(offset.at(frameIdx).first) + "," + std::to_string(offset.at(frameIdx).second) + " -> " + std::to_string(frame.width()) + "x" + std::to_string(frame.height());

                    PngImage image(frame.width(), frame.height());
                    for (uint16_t y = 0, h = frame.height(); y < h; y++) {
                        for (uint16_t x = 0, w = frame.width(); x < w; x++) {
                            const Falltergeist::Format::Pal::Color* color = pal.color(frame.index(x, y));
                            image.setPixel(x, y, color->red() * rgbMultiplier, color->green() * rgbMultiplier, color->blue() * rgbMultiplier, color->alpha());
                        }
                    }

                    png.writeAnimFrame( image, offset.at(frameIdx).first, offset.at(frameIdx).second, 0, frm.framesPerSecond() / 2, PNG_DISPOSE_OP_BACKGROUND, PNG_BLEND_OP_SOURCE );

                    frameIdx++;
                }

                png.writeAnimEnd();
                dirIdx++;
                verbose << -1;
            }

            verbose << -1 << "end generator = apng";
        }
        else
        {
            std::cout << "Unknown generator: '" << options.Generator << "'" << std::endl;
            return EXIT_FAILURE;
        }
    }
    catch( std::exception& e )
    {
        verbose << "exception";

        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    verbose << -1 << "exit main block";


    return EXIT_SUCCESS;
}

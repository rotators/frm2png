/*
 * Copyright (c) 2015-2018 Falltergeist developers
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

// C++ standard includes
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// frm2png includes
#include "ColorPal.h"
#include "PngGenerator.h"

// falltergeist includes
#include "Format/Dat/Stream.h"
#include "Format/Frm/File.h"
#include "Format/Pal/File.h"

// third party includes
#include <clipp.h>

using namespace frm2png;

struct Options
{
    clipp::group CommandLine;

    bool Help    = false;
    bool Version = false;
    bool Info    = false;

    // input
    std::string              DatFile;
    std::string              PalFile;
    std::string              PalName = "default";
    std::vector<std::string> FrmFile;

    // output
    std::string Generator = "auto";
    std::string PngFile;

    // misc
    bool Verbose = false;

    Options()
    {}

    void Init()
    {
        // clang-format off

        auto cmdInfo =
        (
            clipp::option( "--help", "-h" ).set( Help ).doc( "show help summary" ) |
            clipp::option( "--version", "-v" ).set( Version ).doc( "show program version" )
        )
        .doc( "General options" );

        auto cmdInput =
        (
        //  (clipp::option( "-d", "--dat" ) & clipp::value( "DAT", DatFile )).doc( "Use specified DAT file" ),
            (clipp::option( "-p", "--pal" ) & clipp::value( "PAL", PalFile )).doc( "Use specified PAL file" ) |
            (clipp::option( "-P", "--palette" ) & clipp::value( "name", PalName )).doc( "Use embedded palette" )
        )
        .doc( "Input options" );

        auto cmdOutput =
        (
            (clipp::option( "-g", "--generator" ) & clipp::value( "name", Generator )).doc( "generator" ),
            (clipp::option( "-o", "--output" ) & clipp::value( "PNG", PngFile )).doc( "output filename" )
        )
        .doc( "Output options" );

        auto cmdMisc =
        (
            clipp::option( "-V", "--verbose" ).set( Verbose ).doc( "prints various debug messages" ),
            clipp::option( "-i", "--info" ).set( Info ).doc( "prints FRM info only (doesn't process files)" )
        )
        .doc( "Misc options" );

        // clang-format on

        CommandLine = ( cmdInfo | ( cmdInput, cmdOutput, cmdMisc, clipp::values( "filename.frm", FrmFile ) ) );
    }
};

static void printVersion()
{
    std::cout << "FRM to PNG converter v0.1.5r" << std::endl
              << "Copyright (c) 2015-2018 Falltergeist developers" << std::endl
              << "Copyright (c) 2019-2021 Rotators" << std::endl
              << std::endl;
}

static int exitVersion()
{
    printVersion();

    return EXIT_SUCCESS;
}

static int exitHelp( int exitCode, Options& options )
{
    printVersion();

    auto cmdFormat = clipp::doc_formatting {}
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

static void printFRM( const std::string& filename, Falltergeist::Format::Frm::File& frm )
{
    std::cout << "=== FRM info ===" << std::endl;
    std::cout << "Filename ..............  " << filename << std::endl;
    std::cout << "Version ................ " << frm.Version << std::endl;
    std::cout << "Frames per second ...... " << frm.FramesPerSecond << std::endl;
    std::cout << "Action frame ........... " << frm.ActionFrame << std::endl;
    std::cout << "Directions ............. " << std::to_string( frm.DirectionsSize() ) << std::endl;
    std::cout << "Frames per direction ... " << frm.FramesPerDirection << std::endl;
}

// <- path/to/file.ext
// -> path/to/
// -> file
// -> .ext
static void splitFilename( const std::string& full, std::string& path, std::string& basename, std::string& extension )
{
    size_t      pos;
    std::string filename;

    pos = full.find_last_of( '/' );
    if( pos != std::string::npos )
    {
        path     = full.substr( 0, pos + 1 );
        filename = full.substr( pos + 1 );

        if( path == "./" )
            path.clear();
    }
    else
    {
        path     = "";
        filename = full;
    }

    pos = filename.find_last_of( '.' );
    if( pos != std::string::npos )
    {
        basename  = filename.substr( 0, pos );
        extension = filename.substr( pos );
    }
    else
    {
        basename  = filename;
        extension = "";
    }
}

template<typename T>
T loadFile( const std::string& filename )
{
    std::ifstream stream;

    stream.open( filename, std::ios_base::in | std::ios_base::binary );
    if( !stream.is_open() )
        throw std::runtime_error( "loadFile() - Can't open input file: " + filename );

    T obj = T( stream );

    return obj;
}

static Falltergeist::Format::Pal::File loadPal( const Options& options )
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

int main( int argc, char** argv )
{
    Options options;
    options.Init();

    auto optionsResult = clipp::parse( argc, argv, options.CommandLine );
    {
        Logging verbose;

        std::string frmFile;
        for( const auto& frm : options.FrmFile )
        {
            if( !frmFile.empty() )
                frmFile += ", ";

            frmFile += frm;
        }

        verbose.Enabled = options.Verbose;
        verbose << "command line" << 1
                << "Help      = " + std::string( options.Help ? "true" : "false" )
                << "Version   = " + std::string( options.Version ? "true" : "false" )
                // input
                << "DatFile   = " + options.DatFile
                << "PalFile   = " + options.PalFile
                << "PalName   = " + options.PalName
                << "FrmFile   = " + frmFile
                // output
                << "Generator = " + options.Generator
                << "PngFile   = " + options.PngFile
                // misc
                << "Info      = " + std::string( options.Info ? "true" : "false" )
                << "Verbose   = " + std::string( options.Verbose ? "true" : "false" )
                << -1;
    }

    if( !optionsResult )
        return exitHelp( EXIT_FAILURE, options );
    else if( options.Help )
        return exitHelp( EXIT_SUCCESS, options );
    else if( options.Version )
        return exitVersion();

    try
    {
        Logging logVerbose( options.Verbose );

        logVerbose << "init generators" << 1;
        InitPngGenerators();
        for( const auto& vg : Generator )
        {
            logVerbose << vg.first;
        }
        logVerbose << -1;

        logVerbose << "begin frm loop" << 1;
        for( const std::string& frmFile : options.FrmFile )
        {
            PngGeneratorData data( loadFile<Falltergeist::Format::Frm::File>( frmFile ), loadPal( options ) );

            printFRM( frmFile, data.Frm );

            if( options.Info )
                continue;

            // split output filename into few parts; helps generators to modify filename provided by user

            std::string pngFull;

            if( !options.PngFile.empty() )
                pngFull = options.PngFile;
            else
            {
                std::string frmPath, frmBasename, frmExtension;

                splitFilename( frmFile, frmPath, frmBasename, frmExtension );
                pngFull = frmPath + frmBasename + ".png";
            }

            splitFilename( pngFull, data.PngPath, data.PngBasename, data.PngExtension );

            logVerbose << "png = path[" + data.PngPath + "] basename[" + data.PngBasename + "] extension[" + data.PngExtension + "]";

            // TODO? make rgbMultiplier configurable
            data.Pal.RGBMultiplier( 4 ); // noon

            // select and run .png generator
            std::string generator = options.Generator;
            if( generator == "auto" )
            {
                if( data.Frm.DirectionsSize() == 1 )
                {
                    if( data.Frm.FramesPerDirection == 1 )
                        generator = "legacy";
                    else
                        generator = "anim";
                }
                else
                    generator = "anim";

                logVerbose << "selected generator = " + generator;
            }
            else
                logVerbose << "preselected generator = " + generator;

            auto itGenerator = Generator.find( generator );
            if( itGenerator == Generator.end() )
            {
                std::cout << "Unknown generator: '" << generator << "'" << std::endl;
                return EXIT_FAILURE;
            }

            logVerbose << "start generator = " + generator << 1;
            itGenerator->second( data, logVerbose );
            logVerbose << -1 << "end generator = " + generator;

        } // foreach .frm

        logVerbose << -1 << "end frm loop";
    }
    catch( std::exception& e )
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

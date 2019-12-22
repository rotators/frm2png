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
#include <string>

// frm2png includes
#include "ColorPal.h"
#include "Exception.h"
#include "PngImage.h"
#include "PngWriter.h"

// falltergeist includes
#include "Format/Dat/Stream.h"
#include "Format/Frm/File.h"

// Third party includes
#include <png.h>

using namespace frm2png;

void usage(const std::string& binaryName)
{
    std::cout << "FRM to PNG converter v0.1.5r" << std::endl;
    std::cout << "Copyright (c) 2015-2018 Falltergeist developers" << std::endl;
    std::cout << "Copyright (c) 2019 Rotators" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: " << binaryName << " <FRM filename>" << std::endl;
}

void frmInfo(const std::string& filename, Falltergeist::Format::Frm::File& frm)
{
    std::cout << "=== FRM info ===" << std::endl;
    std::cout << "Filename ..............  " << filename << std::endl;
    std::cout << "Version ................ " << frm.version() << std::endl;
    std::cout << "Frames per second ...... " << frm.framesPerSecond() << std::endl;
    std::cout << "Action frame ........... " << frm.actionFrame() << std::endl;
    std::cout << "Directions ............. " << frm.directions().size() << std::endl;
    std::cout << "Frames per direction ... " << frm.framesPerDirection() << std::endl;
}

template<typename T>
T streamFile(const std::string& filename)
{
    std::ifstream stream;

    stream.open(filename, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open())
        throw Exception( "openStreamFile() - Can't open input file: " + filename );

    T obj = T( stream );

    return obj;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    std::string filename = argv[1];
    std::string basename = filename.substr(0, filename.find('.'));
    std::replace(basename.begin(), basename.end(), '\\','/');

    bool apng = false;
    if (argc >= 3 && std::string(argv[2]) == "apng")
        apng = true;

    try {
        auto frm = streamFile<Falltergeist::Format::Frm::File>(filename);
        frmInfo(filename, frm);

        // TODO allow setting .pal file from command line
        //auto pallete = streamFile<Falltergeist::Format::Frm::File>("color.pal"));
        Falltergeist::Format::Pal::File pallete( ColorPal["default"] );

        // TODO decide what to do with Frm::File::rbga() and Frm::File::mask()
        // they're most likely used by falltergeist graphics engine, and kinda pointless to keep here
        //frm.rgba(&pallete);
        //frm.mask(&pallete);

        // TODO? make rgbMultiplier configurable
        static constexpr uint8_t rgbMultiplier = 4;

        if (apng && frm.directions().size() == 1)
            apng = false;

        if (!apng)
        {
            // find maximum width and height

            uint16_t maxWidth = frm.directions().front().frames().front().width();
            uint16_t maxHeight = frm.directions().front().frames().front().height();
            for (const auto& dir : frm.directions()) {
                for (const auto& frame : dir.frames()) {
                    if (frame.width() > maxWidth) maxWidth = frame.width();
                    if (frame.height() > maxHeight) maxHeight = frame.height();
                }
            }

            // draw .png

            PngImage image(maxWidth*frm.framesPerDirection(), maxHeight*frm.directions().size());

            uint8_t  dirIdx = 0;
            for (const auto& dir : frm.directions()) {
                uint16_t frameIdx = 0;
                for (const auto& frame : dir.frames()) {
                    const uint32_t pngX = maxWidth * frameIdx;
                    const uint32_t pngY = maxHeight * dirIdx;

                    for (uint16_t y = 0, h = frame.height(); y < h; y++) {
                        for (uint16_t x = 0, w = frame.width(); x < w; x++) {
                            const Falltergeist::Format::Pal::Color* color = pallete.color(frame.index(x, y));
                            image.setPixel(pngX + x, pngY + y, color->red() * rgbMultiplier, color->green()  * rgbMultiplier, color->blue()  * rgbMultiplier, color->alpha());
                        }
                    }
                    frameIdx++;
                }
                dirIdx++;
            }

            PngWriter png(basename + ".png");
            png.write(image);
        }
        else // experimental apng support
        {
            uint8_t dirIdx = 0;
            for (const auto& dir : frm.directions()) {
                // TODO support acTL + fcTL + IDAT variant
                static constexpr bool firstIsAnim = false;

                // .frm frames iteration [1/3]; init conversion from .frm offsets to .png offsets

                uint32_t ihdrWidth = 0, ihdrHeight = 0;
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

                    ihdrWidth = std::max<uint32_t>(ihdrWidth, frameOffset.first + dir.frames().at(frameIdx).width());
                    ihdrHeight = std::max<uint32_t>(ihdrHeight, frameOffset.second + dir.frames().at(frameIdx).height());

                    frameIdx++;
                }

                PngWriter png(basename + "_" + std::to_string(dirIdx) + ".png");

                png.writeAnimHeader( ihdrWidth, ihdrHeight, dir.frames().size() + (firstIsAnim ? 0 : 1), 0, !firstIsAnim );

                if (!firstIsAnim) {
                    // if first image is not supposed to be part of animation, copy of first frame is added and moved to center
                    // software supporting APNG will ignore it, anything else will use that as image to display

                    auto& frame = dir.frames().front();
                    PngImage defaultImage(ihdrWidth, ihdrHeight);

                    // draw .png frame

                    uint16_t pngX = ihdrWidth / 2 - frame.width() / 2;
                    uint16_t pngY = ihdrHeight / 2 - frame.height() / 2;
                    for (uint16_t x = 0, w = frame.width(); x < w; x++) {
                        for (uint16_t y = 0, h = frame.height(); y < h; y++) {
                            const Falltergeist::Format::Pal::Color* color = pallete.color(frame.index(x, y));
                            defaultImage.setPixel(pngX + x, pngY + y, color->red() * rgbMultiplier, color->green() * rgbMultiplier, color->blue() * rgbMultiplier, color->alpha());
                        }
                    }

                    png.writeAnimFrame( defaultImage, 0, 0, 0, 0, 0, 0 );
                }

                // .frm frames iteration [3/3]; draw .png frames
                frameIdx = 0;
                for (const auto& frame : dir.frames()) {

                    // draw .png frame

                    PngImage image(frame.width(), frame.height());
                    for (uint16_t y = 0, h = frame.height(); y < h; y++) {
                        for (uint16_t x = 0, w = frame.width(); x < w; x++) {
                            const Falltergeist::Format::Pal::Color* color = pallete.color(frame.index(x, y));
                            image.setPixel(x, y, color->red() * rgbMultiplier, color->green() * rgbMultiplier, color->blue() * rgbMultiplier, color->alpha());
                        }
                    }

                    png.writeAnimFrame( image, offset.at(frameIdx).first, offset.at(frameIdx).second, 0, frm.framesPerSecond() / 2, PNG_DISPOSE_OP_BACKGROUND, PNG_BLEND_OP_SOURCE );

                    frameIdx++;
                }

                png.writeAnimEnd();
                dirIdx++;
            }
        }

    } catch(Exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

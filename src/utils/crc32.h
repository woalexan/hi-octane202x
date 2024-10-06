/* This source code was taken/translated to C++ by myself from the following website (original authors site)
 * // http://sanity-free.org/12/crc32_implementation_in_csharp.html
 
 This website does not mention any license here. The original author has also created a Github
 project for Crc calculations, from which I have copied the license text here (even though my code
 is derived from the authors website and not from the Github project)

//
// Crc32.cs
//
// Author:
//       steve whitley <steve@nullfx.com>
//
// Copyright (c) 2017 
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.                                                                            */

#ifndef CRC32_H
#define CRC32_H

#define CRC32_TABLELEN 256

#include <vector>
#include <cstdint>

int16_t ConvertByteArray_ToInt16(std::vector<unsigned char> bytes, unsigned int start_position);
void ConvertAndWriteInt16ToByteArray(int inputValue, std::vector<unsigned char> &bytes, unsigned int writeIndex);

class Crc32 {
public:
    Crc32();

    unsigned int m_table[CRC32_TABLELEN];
    unsigned int ComputeChecksum(std::vector<unsigned char> bytes);

    //not implemented yet, maybe not needed public byte[] ComputeChecksumBytes(byte[] bytes);
};

#endif // CRC32_H

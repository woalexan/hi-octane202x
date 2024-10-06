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

#include "crc32.h"

Crc32::Crc32() {
    unsigned int poly = 0xedb88320;
    unsigned int temp = 0;

    for (unsigned int i = 0; i < CRC32_TABLELEN; ++i) {
        temp = i;
        for (int j = 8; j > 0; --j) {
            if ((temp & 1) == 1) {
                temp = (unsigned int)((temp >> 1) ^ poly);
             } else {
                temp >>= 1;
               }
        }
        m_table[i] = temp;
    }
}

unsigned int Crc32::ComputeChecksum(std::vector<unsigned char> bytes) {
  unsigned int crc = 0xffffffff;
  unsigned long len = bytes.size();
  unsigned char index;

  for (unsigned long i = 0; i < len; ++i) {
        index = (unsigned char)(((crc) & 0xff) ^ bytes.at(i));
        crc = (unsigned int)((crc >> 8) ^ m_table[index]);
    }

  return ~crc;
}

int16_t ConvertByteArray_ToInt16(std::vector<unsigned char> bytes, unsigned int start_position) {
    int16_t result;

    result = static_cast<int16_t>((bytes.at(start_position+1) << 8) + bytes.at(start_position));
    return (result);
}

void ConvertAndWriteInt16ToByteArray(int inputValue, std::vector<unsigned char> &bytes, unsigned int writeIndex) {
    int16_t inValue = static_cast<int16_t>(inputValue);

    bytes.at(writeIndex+1) = static_cast<uint8_t>(inValue & 0xFF00) >> 8;
    bytes.at(writeIndex) = static_cast<uint8_t>(inValue & 0x00FF);
}


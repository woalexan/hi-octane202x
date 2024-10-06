/*
 * OPL Bank Editor by Wohlstand, a free tool for music bank editing
 * Copyright (c) 2016-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * OPN2 Bank Editor by Wohlstand, a free tool for music bank editing
 * Copyright (c) 2017-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Note Wolf Alexander: The source code below is not 100% the original code from the
 * initial author anymore, as I had to make changes to it to be able to compile it
 * on my system. */

#include "common.h"
#include <cstring>

int64_t readLE(std::ifstream &file, uint16_t &out)
{
    uint8_t bytes[2] = {0, 0};
    file.read(char_p(bytes), 2);
    int64_t len = file.gcount();
    out = (uint16_t(bytes[0]) & 0x00FF)
          | ((uint16_t(bytes[1]) << 8) & 0xFF00);
    return len;
}


int64_t readLE(std::ifstream &file, uint32_t &out)
{
    uint8_t bytes[4] = {0, 0, 0, 0};
    file.read(char_p(bytes), 4);
    int64_t len = file.gcount();
    out =    (uint32_t(bytes[0]  << 0) & 0x000000FF)
          | ((uint32_t(bytes[1]) << 8)  & 0x0000FF00)
          | ((uint32_t(bytes[2]) << 16) & 0x00FF0000)
          | ((uint32_t(bytes[3]) << 24) & 0xFF000000);
    return len;
}

int64_t writeLE(std::ofstream &file, const uint16_t &in)
{
   uint8_t bytes[2] = {uint8_t(in & 0x00FF), uint8_t((in >> 8) & 0x00FF) };
   size_t before = file.tellp(); //current pos
   file.write(char_p(bytes), 2);
   size_t after = file.tellp();
   int64_t len = after - before;

   return len;
}


int64_t writeLE(std::ofstream &file, const uint32_t &in)
{
    uint8_t bytes[4] = { uint8_t(in & 0x000000FF),
                         uint8_t((in >> 8) & 0x000000FF),
                         uint8_t((in >> 16) & 0x000000FF),
                         uint8_t((in >> 24) & 0x000000FF)
                       };
    size_t before = file.tellp(); //current pos
    file.write(char_p(bytes), 4);
    size_t after = file.tellp();
    int64_t len = after - before;

    return len;
}


int64_t readBE(std::ifstream &file, uint16_t &out)
{
    uint8_t bytes[2] = {0, 0};
    file.read(char_p(bytes), 2);
    int64_t len = file.gcount();
    out = (uint16_t(bytes[1]) & 0x00FF)
          | ((uint16_t(bytes[0]) << 8) & 0xFF00);
    return len;
}

int64_t writeBE(std::ofstream &file, const uint16_t &in)
{
    uint8_t bytes[2] = {uint8_t((in >> 8) & 0x00FF), uint8_t(in & 0x00FF)};
    size_t before = file.tellp(); //current pos
    file.write(char_p(bytes), 2);
    size_t after = file.tellp();
    int64_t len = after - before;
    return len;
}

int16_t toSint16LE(const uint8_t *arr)
{
    int16_t num = *reinterpret_cast<const int8_t *>(&arr[1]);
    num *= 1 << 8;
    num |= arr[0];
    return num;
}

uint16_t toUint16LE(const uint8_t *arr)
{
    uint16_t num = arr[0];
    num |= ((arr[1] << 8) & 0xFF00);
    return num;
}

uint16_t toUint16BE(const uint8_t *arr)
{
    uint16_t num = arr[1];
    num |= ((arr[0] << 8) & 0xFF00);
    return num;
}


int16_t toSint16BE(const uint8_t *arr)
{
    int16_t num = *reinterpret_cast<const int8_t *>(&arr[0]);
    num *= 1 << 8;
    num |= arr[1];
    return num;
}

uint32_t toUint32LE(const uint8_t *arr)
{
    uint32_t num = arr[0];
    num |= (static_cast<uint32_t>(arr[1] << 8)  & 0x0000FF00);
    num |= (static_cast<uint32_t>(arr[2] << 16) & 0x00FF0000);
    num |= (static_cast<uint32_t>(arr[3] << 24) & 0xFF000000);
    return num;
}

uint32_t toUint32BE(const uint8_t *arr)
{
    uint32_t num = arr[3];
    num |= (static_cast<uint32_t>(arr[2] << 8)  & 0x0000FF00);
    num |= (static_cast<uint32_t>(arr[1] << 16) & 0x00FF0000);
    num |= (static_cast<uint32_t>(arr[0] << 24) & 0xFF000000);
    return num;
}


void fromSint16LE(int16_t in, uint8_t *arr)
{
    arr[0] =  in & 0x00FF;
    arr[1] = (in >> 8) & 0x00FF;
}

void fromUint16LE(uint16_t in, uint8_t *arr)
{
    arr[0] =  in & 0x00FF;
    arr[1] = (in >> 8) & 0x00FF;
}

void fromUint16BE(uint16_t in, uint8_t *arr)
{
    arr[1] =  in & 0x00FF;
    arr[0] = (in >> 8) & 0x00FF;
}

void fromSint16BE(int16_t in, uint8_t *arr)
{
    arr[1] =  in & 0x00FF;
    arr[0] = (in >> 8) & 0x00FF;
}

void fromUint32LE(uint32_t in, uint8_t *arr)
{
    arr[0] = (in)     & 0x00FF;
    arr[1] = (in >> 8)  & 0x00FF;
    arr[2] = (in >> 16) & 0x00FF;
    arr[3] = (in >> 24) & 0x00FF;
}


void getMagic(std::string filePath, char *bytes, int count)
{
    std::ifstream file( filePath, std::ios::binary );

    memset(bytes, 0, static_cast<size_t>(count));

    if(!file.fail())
    {
        file.read(bytes, count);
        file.close();
    }
}

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool hasExt(const std::string &file, const char *ext)
{
    return hasEnding(file, ext);
}

uint8_t clip_u8(int i, int min, int max)
{
    if(i < min)
        return min;
    else if(i > max)
        return max;
    else
        return static_cast<uint8_t>(i);
}



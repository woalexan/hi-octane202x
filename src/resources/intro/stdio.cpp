// Aseprite FLIC Library
// Copyright (c) 2015 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

//Note Wolf Alexander:  for original authors license file please see flic-LICENSE.txt
//Source code taken from https://github.com/aseprite/flic

#include "flic.h"

namespace flic {

StdioFileInterface::StdioFileInterface(FILE* file)
  : m_file(file)
  , m_ok(true)
{
}

bool StdioFileInterface::ok() const
{
  return m_ok;
}

size_t StdioFileInterface::tell()
{
  return (size_t)(ftell(m_file));
}

void StdioFileInterface::seek(size_t absPos)
{
  fseek(m_file, (long)(absPos), SEEK_SET);
}

uint8_t StdioFileInterface::read8()
{
  int value = fgetc(m_file);
  if (value != EOF)
    return value;

  m_ok = false;
  return 0;
}

void StdioFileInterface::write8(uint8_t value)
{
  fputc(value, m_file);
}

} // namespace flic

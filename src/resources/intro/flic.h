// Aseprite FLIC Library
// Copyright (c) 2015 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

//Note Wolf Alexander:  for original authors license file please see flic-LICENSE.txt
//Source code taken from https://github.com/aseprite/flic

//Note 22.03.2025: In an attempt to reduce/remove the warnings due to type conversions and uninitialized
// variables under Visual Studio, I decided to modify some parts of the original code below. I also replaced
// NULL with nullptr.
//Therefore the source code below is not the original source code of the original author anymore.

#ifndef FLIC_FLIC_H_INCLUDED
#define FLIC_FLIC_H_INCLUDED
#pragma once

#include <stdint.h>

#include <cassert>
#include <cstdio>
#include <vector>

namespace flic {

  struct Color {
    uint8_t r, g, b;

    Color() : r(0), g(0), b(0) {
    }

    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {
    }

    bool operator==(const Color& o) const {
      return r == o.r && g == o.g && b == o.b;
    }

    bool operator!=(const Color& o) const {
      return !operator==(o);
    }
  };

  struct Header {
    int frames;
    int width;
    int height;
    int speed;
  };

  class Colormap {
  public:
    static const int SIZE = 256;

    Colormap() {
    }

    int size() const {
      return SIZE;
    }

    const Color& operator[](int i) const {
      assert(i >= 0 && i < SIZE);
      return m_color[i];
    }

    Color& operator[](int i) {
      assert(i >= 0 && i < SIZE);
      return m_color[i];
    }

    bool operator==(const Colormap& o) const {
      for (int i=0; i<SIZE; ++i) {
        if (m_color[i] != o[i])
          return false;
      }
      return true;
    }

    bool operator!=(const Colormap& o) const {
      return !operator==(o);
    }

  private:
    Color m_color[SIZE];
  };

  struct Frame {
    uint8_t* pixels = nullptr;
    uint32_t rowstride;
    Colormap colormap;
  };

  class FileInterface {
  public:
    virtual ~FileInterface() { }

    // Returns true if we can read/write bytes from/into the file
    virtual bool ok() const = 0;

    // Current position in the file
    virtual size_t tell() = 0;

    // Jump to the given position in the file
    virtual void seek(size_t absPos) = 0;

    // Returns the next byte in the file or 0 if ok() = false
    virtual uint8_t read8() = 0;

    // Writes one byte in the file (or do nothing if ok() = false)
    virtual void write8(uint8_t value) = 0;
  };

  class StdioFileInterface : public flic::FileInterface {
  public:
    StdioFileInterface(FILE* file);
    bool ok() const override;
    size_t tell() override;
    void seek(size_t absPos) override;
    uint8_t read8() override;
    void write8(uint8_t value) override;

  private:
    FILE* m_file = nullptr;
    bool m_ok;
  };

  class Decoder {
  public:
    Decoder(FileInterface* file);
    bool readHeader(Header& header);
    bool readFrame(Frame& frame);

  private:
    void readChunk(Frame& frame);
    void readBlackChunk(Frame& frame);
    void readCopyChunk(Frame& frame);
    void readColorChunk(Frame& frame, bool oldColorChunk);
    void readBrunChunk(Frame& frame);
    void readLcChunk(Frame& frame);
    void readDeltaChunk(Frame& frame);
    uint16_t read16();
    uint32_t read32();

    FileInterface* m_file = nullptr;
    int m_width = 0, m_height = 0;
    int m_frameCount = 0;
    int m_offsetFrame1 = 0;
    int m_offsetFrame2 = 0;
  };

  class Encoder {
  public:
    Encoder(FileInterface* file);
    ~Encoder();

    void writeHeader(const Header& header);
    void writeFrame(const Frame& frame);

    // Must be called at the end with the first frame. It's required
    // by Animator Pro to loop the animation from the last frame to
    // the first one.
    void writeRingFrame(const Frame& frame);

  private:
    void writeColorChunk(const Frame& frame);
    void writeBrunChunk(const Frame& frame);
    void writeBrunLineChunk(const Frame& frame, int y);
    void writeLcChunk(const Frame& frame);
    void writeLcLineChunk(const Frame& frame, int y);
    void write16(uint16_t value);
    void write32(uint32_t value);

    FileInterface* m_file = nullptr;
    int m_width = 0, m_height = 0;
    Colormap m_prevColormap;
    std::vector<uint8_t> m_prevFrameData;
    int m_frameCount = 0;
    uint32_t m_offsetFrame1 = 0;
    uint32_t m_offsetFrame2 = 0;
  };

} // namespace flic

#endif

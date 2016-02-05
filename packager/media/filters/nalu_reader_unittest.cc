// Copyright 2016 Google Inc. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include <gtest/gtest.h>

#include "packager/media/filters/nalu_reader.h"

namespace edash_packager {
namespace media {

TEST(NaluReaderTest, StartCodeSearch) {
  const uint8_t kNaluData[] = {
    0x01, 0x00, 0x00, 0x04, 0x23, 0x56,
    // First NALU
    0x00, 0x00, 0x01, 0x12, 0x34, 0x56, 0x78,
    // Second NALU
    0x00, 0x00, 0x00, 0x01, 0x67, 0xbb, 0xcc, 0xdd
  };

  NaluReader reader(kIsAnnexbByteStream, kNaluData, arraysize(kNaluData));

  Nalu nalu;
  ASSERT_EQ(NaluReader::kOk, reader.Advance(&nalu));
  EXPECT_EQ(kNaluData + 6, nalu.data());
  EXPECT_EQ(3u, nalu.data_size());
  EXPECT_EQ(4u, nalu.header_size());
  EXPECT_EQ(0, nalu.ref_idc());
  EXPECT_EQ(0x12, nalu.type());

  ASSERT_EQ(NaluReader::kOk, reader.Advance(&nalu));
  EXPECT_EQ(kNaluData + 13, nalu.data());
  EXPECT_EQ(3u, nalu.data_size());
  EXPECT_EQ(5u, nalu.header_size());
  EXPECT_EQ(3, nalu.ref_idc());
  EXPECT_EQ(7, nalu.type());

  EXPECT_EQ(NaluReader::kEOStream, reader.Advance(&nalu));
}

TEST(NaluReaderTest, OneByteNaluLength) {
  const uint8_t kNaluData[] = {
    // First NALU
    0x05, 0x08, 0x01, 0x02, 0x03, 0x04,
    // Second NALU
    0x06, 0x67, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e
  };

  NaluReader reader(1, kNaluData, arraysize(kNaluData));

  Nalu nalu;
  ASSERT_EQ(NaluReader::kOk, reader.Advance(&nalu));
  EXPECT_EQ(kNaluData, nalu.data());
  EXPECT_EQ(4u, nalu.data_size());
  EXPECT_EQ(2u, nalu.header_size());
  EXPECT_EQ(0, nalu.ref_idc());
  EXPECT_EQ(8, nalu.type());

  ASSERT_EQ(NaluReader::kOk, reader.Advance(&nalu));
  EXPECT_EQ(kNaluData + 6, nalu.data());
  EXPECT_EQ(5u, nalu.data_size());
  EXPECT_EQ(2u, nalu.header_size());
  EXPECT_EQ(3, nalu.ref_idc());
  EXPECT_EQ(7, nalu.type());

  EXPECT_EQ(NaluReader::kEOStream, reader.Advance(&nalu));
}

TEST(NaluReaderTest, ThreeByteNaluLength) {
  const uint8_t kNaluData[] = {
    // First NALU
    0x00, 0x00, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    // Second NALU
    0x00, 0x00, 0x03, 0x67, 0x0a, 0x0b
  };

  NaluReader reader(3, kNaluData, arraysize(kNaluData));

  Nalu nalu;
  ASSERT_EQ(NaluReader::kOk, reader.Advance(&nalu));
  EXPECT_EQ(kNaluData, nalu.data());
  EXPECT_EQ(6u, nalu.data_size());
  EXPECT_EQ(4u, nalu.header_size());
  EXPECT_EQ(0, nalu.ref_idc());
  EXPECT_EQ(8, nalu.type());

  ASSERT_EQ(NaluReader::kOk, reader.Advance(&nalu));
  EXPECT_EQ(kNaluData + 10, nalu.data());
  EXPECT_EQ(2u, nalu.data_size());
  EXPECT_EQ(4u, nalu.header_size());
  EXPECT_EQ(3, nalu.ref_idc());
  EXPECT_EQ(7, nalu.type());

  EXPECT_EQ(NaluReader::kEOStream, reader.Advance(&nalu));
}

TEST(NaluReaderTest, ErrorForNotEnoughForNaluLength) {
  const uint8_t kNaluData[] = {
    // First NALU
    0x00
  };

  NaluReader reader(3, kNaluData, arraysize(kNaluData));

  Nalu nalu;
  EXPECT_EQ(NaluReader::kInvalidStream, reader.Advance(&nalu));
}

TEST(NaluReaderTest, ErrorForNaluLengthExceedsRemainingData) {
  const uint8_t kNaluData[] = {
    // First NALU
    0xFF, 0x08, 0x00
  };

  NaluReader reader(1, kNaluData, arraysize(kNaluData));

  Nalu nalu;
  EXPECT_EQ(NaluReader::kInvalidStream, reader.Advance(&nalu));

  // Another test for off by one.
  const uint8_t kNaluData2[] = {
    // First NALU
    0x04, 0x08, 0x00, 0x00
  };

  NaluReader reader2(1, kNaluData2, arraysize(kNaluData2));
  EXPECT_EQ(NaluReader::kInvalidStream, reader2.Advance(&nalu));
}

TEST(NaluReaderTest, ErrorForForbiddenBitSet) {
  const uint8_t kNaluData[] = {
    // First NALU
    0x03, 0x80, 0x00, 0x00
  };

  NaluReader reader(1, kNaluData, arraysize(kNaluData));

  Nalu nalu;
  EXPECT_EQ(NaluReader::kInvalidStream, reader.Advance(&nalu));
}

}  // namespace media
}  // namespace edash_packager
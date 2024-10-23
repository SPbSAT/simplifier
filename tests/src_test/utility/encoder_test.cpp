#include "src/utility/logger.hpp"
#include "src/utility/encoder.hpp"

#include <string>

#include "gtest/gtest.h"

namespace
{

using namespace csat;
using namespace csat::utils;

Logger logger("EncoderTest");

TEST(EncoderTest, EncodeDecode)
{
    logger.info("Testing simple encoding/decoding");
    
    GateEncoder enc;
    enc.encodeGate("a");
    enc.encodeGate("b");
    enc.encodeGate("c");
    enc.encodeGate("d");
    enc.encodeGate("e");
    
    ASSERT_TRUE(enc.size() == 5);
    
    ASSERT_TRUE(enc.encodeGate("a") == 0);
    ASSERT_TRUE(enc.encodeGate("b") == 1);
    ASSERT_TRUE(enc.encodeGate("c") == 2);
    ASSERT_TRUE(enc.encodeGate("d") == 3);
    ASSERT_TRUE(enc.encodeGate("e") == 4);
    
    ASSERT_TRUE(enc.decodeGate(0) == "a");
    ASSERT_TRUE(enc.decodeGate(1) == "b");
    ASSERT_TRUE(enc.decodeGate(2) == "c");
    ASSERT_TRUE(enc.decodeGate(3) == "d");
    ASSERT_TRUE(enc.decodeGate(4) == "e");
    
}

} // namespace

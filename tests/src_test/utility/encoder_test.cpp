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
    
    GateEncoder<std::string> enc;
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


TEST(EncoderTest, Merge)
{
    logger.info("Testing simple encoder merging cases");
    
    GateEncoder<std::string> first;
    first.encodeGate("a");
    first.encodeGate("b");
    first.encodeGate("c");
    first.encodeGate("d");
    first.encodeGate("e");
    
    GateEncoder<GateId> second;
    second.encodeGate(0);
    second.encodeGate(2);
    second.encodeGate(3);
    
    auto merged = mergeGateEncoders(first, second);
    
    ASSERT_TRUE(merged->size() == 3);

    ASSERT_TRUE(merged->decodeGate(0) == "a");
    ASSERT_TRUE(merged->encodeGate("a") == second.encodeGate(first.encodeGate("a")));
    
    ASSERT_TRUE(merged->decodeGate(1) == "c");
    ASSERT_TRUE(merged->encodeGate("c") == second.encodeGate(first.encodeGate("c")));
    
    ASSERT_TRUE(merged->decodeGate(2) == "d");
    ASSERT_TRUE(merged->encodeGate("d") == second.encodeGate(first.encodeGate("d")));
}

} // anonymous namespace

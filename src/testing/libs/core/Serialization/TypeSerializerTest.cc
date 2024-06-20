// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"

#include "Serialization/TypeSerializer.h"
#include "Serialization/ReadBuffer.h"
#include "Serialization/WriteBuffer.h"

namespace V8App
{
    namespace Serialization
    {
        /**
         * Helper to make it simplier to call the serializer
         */
        template <typename T>
        bool CallSerializer(BaseBuffer &inBuffer, T &inValue)
        {
            return TypeSerializer<T>::Serialize(inBuffer, inValue);
        }

        namespace internal
        {
            bool EnableByteSwap(bool inBigEndianess)
            {
                return (inBigEndianess && (std::endian::native == std::endian::big)) ||
                       (inBigEndianess == false && (std::endian::native != std::endian::big));
            }
        }
        /**
         * Writer that enables byte swapping
         */
        class SwappingWriteBuffer : public WriteBuffer
        {
        public:
            SwappingWriteBuffer(size_t inInitialSize = 131072) : WriteBuffer(inInitialSize)
            {
                m_StreamState.m_BigEndian = internal::EnableByteSwap(m_StreamState.m_BigEndian);
            }
            SwappingWriteBuffer(const char *inBuffer, size_t inSize) : WriteBuffer(inBuffer, inSize)
            {
                m_StreamState.m_BigEndian = internal::EnableByteSwap(m_StreamState.m_BigEndian);
            }
        };

        /**
         * Reader that enables byte swapping
         */
        class SwappingReadBuffer : public ReadBuffer
        {
        public:
            SwappingReadBuffer(size_t inInitialSize = 131072) : ReadBuffer(inInitialSize)
            {
                m_StreamState.m_BigEndian = internal::EnableByteSwap(m_StreamState.m_BigEndian);
            }
            SwappingReadBuffer(const char *inBuffer, size_t inSize) : ReadBuffer(inBuffer, inSize)
            {
                m_StreamState.m_BigEndian = internal::EnableByteSwap(m_StreamState.m_BigEndian);
            }
        };

        TEST(TypeSerializerTest, SwapBytes)
        {
            bool testBool = true;

            int8_t valueByte = 8;
            int8_t testByte = valueByte;
            int8_t testSwapped = valueByte;

            int16_t value2Byte = -21829;
            int16_t test2Byte = value2Byte;
            int16_t test2Swapped = -17494;

            int32_t value4Byte = -1430541637L;
            int32_t test4Byte = value4Byte;
            int32_t test4Swapped = -1146438742L;

            int64_t value8Byte = -6144129543616877893LL;
            int64_t test8Byte = value8Byte;
            int64_t test8Swapped = -4923916900608853078LL;

            float valueFloat = 100;
            float testFloat = valueFloat;
            float testFSwapped = 7.1838967E-41;

            double valueDouble = 100;
            double testDouble = valueDouble;
            double testDSwapped = 1.1288411876180801e-319;

            // bool
            SwapBytes(testBool);
            EXPECT_TRUE(testBool);

            // swap byte
            SwapBytes(valueByte);
            EXPECT_EQ(testSwapped, valueByte);
            // swap it back
            SwapBytes(valueByte);
            EXPECT_EQ(testByte, valueByte);

            // swap 2 bytes
            SwapBytes(value2Byte);
            EXPECT_EQ(test2Swapped, value2Byte);
            // swap it back
            SwapBytes(value2Byte);
            EXPECT_EQ(test2Byte, value2Byte);

            // swap 4 bytes
            SwapBytes(value4Byte);
            EXPECT_EQ(test4Swapped, value4Byte);
            // swap it back
            SwapBytes(value4Byte);
            EXPECT_EQ(test4Byte, value4Byte);

            // swap 8 bytes
            SwapBytes(value8Byte);
            EXPECT_EQ(test8Swapped, value8Byte);
            // swap it back
            SwapBytes(value8Byte);
            EXPECT_EQ(test8Byte, value8Byte);

            // float
            SwapBytes(valueFloat);
            EXPECT_EQ(testFSwapped, valueFloat);
            // swap it back
            SwapBytes(valueFloat);
            EXPECT_EQ(testFloat, valueFloat);

            // double
            SwapBytes(valueDouble);
            EXPECT_EQ(testDSwapped, valueDouble);
            // swap it back
            SwapBytes(valueDouble);
            EXPECT_EQ(testDouble, valueDouble);
        }

        TEST(TypeSerializerTest, SerializeBool)
        {
            std::unique_ptr<WriteBuffer> wBuffer = std::make_unique<WriteBuffer>();

            bool testData = true;

            EXPECT_TRUE(CallSerializer(*wBuffer, testData));

            EXPECT_EQ(true, (bool)*(wBuffer->GetData()));

            std::unique_ptr<ReadBuffer> rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            testData = false;
            EXPECT_TRUE(CallSerializer(*rBuffer, testData));
            EXPECT_TRUE(testData);
        }

        TEST(TypeSerializerTest, SerializeBoolByteSwapping)
        {
            std::unique_ptr<SwappingWriteBuffer> wBuffer = std::make_unique<SwappingWriteBuffer>();

            bool testData = true;

            EXPECT_TRUE(CallSerializer(*wBuffer, testData));

            EXPECT_EQ(true, (bool)*(wBuffer->GetData()));

            std::unique_ptr<SwappingReadBuffer> rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            testData = false;
            EXPECT_TRUE(CallSerializer(*rBuffer, testData));
            EXPECT_TRUE(testData);
        }

        TEST(TypeSerializerTest, SerializeInt16)
        {
            std::unique_ptr<WriteBuffer> wBuffer = std::make_unique<WriteBuffer>();

            // test signed
            int16_t value = -21829;
            int16_t testSInt = value;

            EXPECT_TRUE(CallSerializer(*wBuffer, value));

            EXPECT_EQ(testSInt, *((int16_t *)(wBuffer->GetData())));

            std::unique_ptr<ReadBuffer> rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testSInt, value);

            // test unsigned
            wBuffer = std::make_unique<WriteBuffer>();
            uint16_t uValue = 43707;
            uint16_t testUCompare = uValue;

            EXPECT_TRUE(CallSerializer(*wBuffer, uValue));
            EXPECT_EQ(testUCompare, *((uint16_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            uValue = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, uValue));
            EXPECT_EQ(testUCompare, uValue);

            // test const value
            wBuffer = std::make_unique<WriteBuffer>();
            const int16_t testCInt = -21829;

            EXPECT_TRUE(CallSerializer(*wBuffer, testCInt));
            EXPECT_EQ(testSInt, *((int16_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCInt));
            // use the non const to see that we can read the const we wrote into it
            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testSInt, value);
        }

        TEST(TypeSerializerTest, SerializeInt16Swapping)
        {
            std::unique_ptr<SwappingWriteBuffer> wBuffer = std::make_unique<SwappingWriteBuffer>();

            // test signed
            int16_t value = -21829;
            int16_t testSInt = value;
            int16_t testSwapped = value;
            SwapBytes(testSwapped);

            EXPECT_TRUE(CallSerializer(*wBuffer, value));
            EXPECT_EQ(testSwapped, *((int16_t *)(wBuffer->GetData())));

            std::unique_ptr<SwappingReadBuffer> rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testSInt, value);

            // test unsigned
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            uint16_t uValue = 43707;
            uint16_t testUInt = uValue;
            uint16_t testUSwapped = uValue;
            SwapBytes(testUSwapped);

            EXPECT_TRUE(CallSerializer(*wBuffer, uValue));
            EXPECT_EQ(testUSwapped, *((uint16_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            uValue = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, uValue));
            EXPECT_EQ(testUInt, uValue);

            // test const
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            const int16_t testCInt = -21829;

            EXPECT_TRUE(CallSerializer(*wBuffer, testCInt));
            EXPECT_EQ(testSwapped, *((int16_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCInt));
            // use the non const to see that we can read the const we wrote into it
            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testSInt, value);
        }

        TEST(TypeSerializerTest, SerializeInt32)
        {
            std::unique_ptr<WriteBuffer> wBuffer = std::make_unique<WriteBuffer>();

            // test signed
            int32_t value = -1430541637;
            int32_t testSInt = value;
            int32_t testCompare = -1430541637;

            EXPECT_TRUE(CallSerializer(*wBuffer, value));
            EXPECT_EQ(testCompare, *((int32_t *)(wBuffer->GetData())));

            std::unique_ptr<ReadBuffer> rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testSInt, value);

            // test unsigned
            wBuffer = std::make_unique<WriteBuffer>();
            uint32_t testUInt = 43707;
            uint32_t testUCompare = testUInt;

            EXPECT_TRUE(CallSerializer(*wBuffer, testUInt));
            EXPECT_EQ(testUCompare, *((uint32_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            testUInt = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, testUInt));
            EXPECT_EQ(testUCompare, testUInt);

            // test const value
            wBuffer = std::make_unique<WriteBuffer>();
            const int32_t testCInt = testSInt;

            EXPECT_TRUE(CallSerializer(*wBuffer, testCInt));
            EXPECT_EQ(testCompare, *((int32_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCInt));
            // use the non const to see that we can read the const we wrote into it
            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testSInt, value);
        }

        TEST(TypeSerializerTest, SerializeInt32Swapping)
        {
            std::unique_ptr<SwappingWriteBuffer> wBuffer = std::make_unique<SwappingWriteBuffer>();

            // test signed
            int32_t value = -1430541637;
            int32_t testSInt = value;
            int32_t testSwapped = value;
            SwapBytes(testSwapped);

            EXPECT_TRUE(CallSerializer(*wBuffer, value));
            EXPECT_EQ(testSwapped, *((int32_t *)(wBuffer->GetData())));

            std::unique_ptr<SwappingReadBuffer> rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testSInt, value);

            // test unsigned
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            uint32_t uValue = 2864425659;
            uint32_t testUInt = uValue;
            uint32_t testUSwapped = uValue;
            SwapBytes(testUSwapped);

            EXPECT_TRUE(CallSerializer(*wBuffer, uValue));
            EXPECT_EQ(testUSwapped, *((uint32_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            uValue = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, uValue));
            EXPECT_EQ(testUInt, uValue);

            // test const
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            const int32_t testCInt = testSInt;

            EXPECT_TRUE(CallSerializer(*wBuffer, testCInt));
            EXPECT_EQ(testSwapped, *((int32_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCInt));
            // use the non const to see that we can read the const we wrote into it
            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testSInt, value);
        }

        TEST(TypeSerializerTest, SerializeInt64)
        {
            std::unique_ptr<WriteBuffer> wBuffer = std::make_unique<WriteBuffer>();

            // test signed
            int64_t value = -6144129543616877893LL;
            int64_t testSInt = value;

            EXPECT_TRUE(CallSerializer(*wBuffer, value));
            EXPECT_EQ(testSInt, *((int64_t *)(wBuffer->GetData())));

            std::unique_ptr<ReadBuffer> rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testSInt, value);

            // test unsigned
            wBuffer = std::make_unique<WriteBuffer>();
            uint64_t uValue = 12302614530092673723ULL;
            uint64_t testUInt = uValue;

            EXPECT_TRUE(CallSerializer(*wBuffer, uValue));
            EXPECT_EQ(testUInt, *((uint64_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            uValue = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, uValue));
            EXPECT_EQ(testUInt, uValue);

            // test const value
            wBuffer = std::make_unique<WriteBuffer>();
            const int64_t testCInt = testSInt;

            EXPECT_TRUE(CallSerializer(*wBuffer, testCInt));
            EXPECT_EQ(testSInt, *((int64_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCInt));
            // use the non const to see that we can read the const we wrote into it
            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testSInt, value);
        }

        TEST(TypeSerializerTest, SerializeInt64Swapping)
        {
            std::unique_ptr<SwappingWriteBuffer> wBuffer = std::make_unique<SwappingWriteBuffer>();

            // test signed
            int64_t value = -6144129543616877893LL;
            int64_t testSInt = value;
            int64_t testSwapped = value;
            SwapBytes(testSwapped);

            EXPECT_TRUE(CallSerializer(*wBuffer, value));
            EXPECT_EQ(-4923916900608853078LL, *((int64_t *)(wBuffer->GetData())));

            std::unique_ptr<SwappingReadBuffer> rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testSInt, value);

            // test unsigned
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            uint64_t uValue = 12302614530092673723ULL;
            uint64_t testUInt = uValue;
            uint64_t testUSwapped = uValue;
            SwapBytes(testUSwapped);

            EXPECT_TRUE(CallSerializer(*wBuffer, uValue));
            EXPECT_EQ(testUSwapped, *((uint64_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            uValue = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, uValue));
            EXPECT_EQ(testUInt, uValue);

            // test const
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            const int64_t testCInt = testSInt;
            EXPECT_TRUE(CallSerializer(*wBuffer, testCInt));
            EXPECT_EQ(testSwapped, *((int64_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCInt));
            // use the non const to see that we can read the const we wrote into it
            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testSInt, value);
        }

        TEST(TypeSerializerTest, SerializeFloat)
        {
            std::unique_ptr<WriteBuffer> wBuffer = std::make_unique<WriteBuffer>();

            // we use hex or we get a rounding error probably from the compiler
            float testValue = 0xAABBAABB;
            float testFloat = testValue;
            float testCompare = 0xAABBAABB;

            EXPECT_TRUE(CallSerializer(*wBuffer, testValue));
            EXPECT_EQ(testCompare, *((float *)(wBuffer->GetData())));

            std::unique_ptr<ReadBuffer> rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            testValue = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, testValue));
            EXPECT_EQ(testFloat, testValue);

            // test const value
            wBuffer = std::make_unique<WriteBuffer>();
            const float testCFloat = 0xAABBAABB;

            EXPECT_TRUE(CallSerializer(*wBuffer, testCFloat));
            EXPECT_EQ(testFloat, *((float *)(wBuffer->GetData())));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCFloat));
            // use the non const to see that we can read the const we wrote into it
            testValue = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, testValue));
            EXPECT_EQ(testFloat, testValue);
        }

        TEST(TypeSerializerTest, SerializeFloatSwapping)
        {
            std::unique_ptr<SwappingWriteBuffer> wBuffer = std::make_unique<SwappingWriteBuffer>();

            // use numbers here since we the hex format is tricky
            float value = 100;
            float testFloat = value;
            float swappedFloat = value;
            SwapBytes(swappedFloat);

            EXPECT_TRUE(CallSerializer(*wBuffer, value));
            EXPECT_EQ(swappedFloat, *((float *)(wBuffer->GetData())));

            std::unique_ptr<SwappingReadBuffer> rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testFloat, value);

            // test const
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            const float testCFloat = testFloat;

            EXPECT_TRUE(CallSerializer(*wBuffer, testCFloat));
            EXPECT_EQ(swappedFloat, *((float *)(wBuffer->GetData())));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCFloat));
            // use the non const to see that we can read the const we wrote into it
            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testFloat, value);
        }

        TEST(TypeSerializerTest, SerializeDouble)
        {
            std::unique_ptr<WriteBuffer> wBuffer = std::make_unique<WriteBuffer>();

            double testValue = 100;
            double testDouble = testValue;
            double testCompare = 100;

            EXPECT_TRUE(CallSerializer(*wBuffer, testValue));
            EXPECT_EQ(testCompare, *((double *)(wBuffer->GetData())));

            std::unique_ptr<ReadBuffer> rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            testValue = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, testValue));
            EXPECT_EQ(testDouble, testValue);

            // test const value
            wBuffer = std::make_unique<WriteBuffer>();
            const double testCDouble = testDouble;

            EXPECT_TRUE(CallSerializer(*wBuffer, testCDouble));
            EXPECT_EQ(testDouble, *((double *)(wBuffer->GetData())));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCDouble));
            // use the non const to see that we can read the const we wrote into it
            testValue = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, testValue));
            EXPECT_EQ(testDouble, testValue);
        }

        TEST(TypeSerializerTest, SerializeDoubleSwapping)
        {
            std::unique_ptr<SwappingWriteBuffer> wBuffer = std::make_unique<SwappingWriteBuffer>();

            double value = 100;
            double testDouble = value;
            double testSwapped = value;
            SwapBytes(testSwapped);

            EXPECT_TRUE(CallSerializer(*wBuffer, value));
            EXPECT_EQ(testSwapped, *((double *)(wBuffer->GetData())));

            std::unique_ptr<SwappingReadBuffer> rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testDouble, value);

            // test const
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            const double testCDouble = testDouble;

            EXPECT_TRUE(CallSerializer(*wBuffer, testCDouble));
            EXPECT_EQ(testSwapped, *((double *)(wBuffer->GetData())));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCDouble));
            // use the non const to see that we can read the const we wrote into it
            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testDouble, value);
        }

        TEST(TypeSerializerTest, SerializeChar)
        {
            std::unique_ptr<WriteBuffer> wBuffer = std::make_unique<WriteBuffer>();

            // test signed
            char value = 'A';
            char testChar = value;
            char testCompare = 'A';

            EXPECT_TRUE(CallSerializer(*wBuffer, value));
            EXPECT_EQ(testCompare, *((char *)(wBuffer->GetData())));

            std::unique_ptr<ReadBuffer> rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testChar, value);

            // test unssigned
            wBuffer = std::make_unique<WriteBuffer>();
            unsigned char uValue = 'A';
            unsigned char testUChar = 'A';

            EXPECT_TRUE(CallSerializer(*wBuffer, uValue));
            EXPECT_EQ(testCompare, *((unsigned char *)(wBuffer->GetData())));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            uValue = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, uValue));
            EXPECT_EQ(testUChar, uValue);

            // test const value
            wBuffer = std::make_unique<WriteBuffer>();
            const char testCChar = testChar;

            EXPECT_TRUE(CallSerializer(*wBuffer, testCChar));
            EXPECT_EQ(testChar, *((const char *)(wBuffer->GetData())));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCChar));
            // use the non const to see that we can read the const we wrote into it
            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testChar, value);
        }

        TEST(TypeSerializerTest, SerializeCharByteSwapping)
        {
            std::unique_ptr<SwappingWriteBuffer> wBuffer = std::make_unique<SwappingWriteBuffer>();

            // test signed
            char value = 'A';
            char testChar = value;
            char testCompare = 'A';

            EXPECT_TRUE(CallSerializer(*wBuffer, value));
            EXPECT_EQ(testCompare, *((char *)(wBuffer->GetData())));

            std::unique_ptr<SwappingReadBuffer> rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testChar, value);

            // test unssigned
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            unsigned char uValue = 'A';
            unsigned char testUChar = uValue;

            EXPECT_TRUE(CallSerializer(*wBuffer, uValue));
            EXPECT_EQ(testUChar, *((unsigned char *)(wBuffer->GetData())));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            uValue = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, uValue));
            EXPECT_EQ(testUChar, uValue);

            // test const value
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            const char testCChar = testChar;

            EXPECT_TRUE(CallSerializer(*wBuffer, testCChar));
            EXPECT_EQ(testChar, *((const char *)(wBuffer->GetData())));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCChar));
            // use the non const to see that we can read the const we wrote into it
            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testChar, value);
        }

        TEST(TypeSerializerTest, SerializeWChar)
        {
            std::unique_ptr<WriteBuffer> wBuffer = std::make_unique<WriteBuffer>();

            wchar_t value = L'A';
            wchar_t testWChar = value;

            EXPECT_TRUE(CallSerializer(*wBuffer, value));
            EXPECT_EQ(testWChar, *((wchar_t *)(wBuffer->GetData())));

            std::unique_ptr<ReadBuffer> rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testWChar, value);

            // test const value
            wBuffer = std::make_unique<WriteBuffer>();
            const wchar_t uValue = testWChar;

            EXPECT_TRUE(CallSerializer(*wBuffer, uValue));
            EXPECT_EQ(testWChar, *((wchar_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, uValue));
            // use the non const to see that we can read the const we wrote into it
            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testWChar, value);
        }

        TEST(TypeSerializerTest, SerializeWCharSwapping)
        {
            std::unique_ptr<SwappingWriteBuffer> wBuffer = std::make_unique<SwappingWriteBuffer>();

            wchar_t value = L'A';
            wchar_t testWChar = value;
            // use the int value of it swapped
            wchar_t testSwapped = 1090519040;

            EXPECT_TRUE(CallSerializer(*wBuffer, value));
            EXPECT_EQ(testSwapped, *((wchar_t *)(wBuffer->GetData())));

            std::unique_ptr<SwappingReadBuffer> rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testWChar, value);

            // test const
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            const wchar_t testCWChar = testWChar;

            EXPECT_TRUE(CallSerializer(*wBuffer, testCWChar));
            EXPECT_EQ(testSwapped, *((wchar_t *)(wBuffer->GetData())));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(CallSerializer(*rBuffer, testCWChar));
            // use the non const to see that we can read the const we wrote into it
            value = 0;
            EXPECT_TRUE(CallSerializer(*rBuffer, value));
            EXPECT_EQ(testWChar, value);
        }

        TEST(TypeSerializerTest, SerializeCharString)
        {
            std::unique_ptr<WriteBuffer> wBuffer = std::make_unique<WriteBuffer>();

            char value[6] = "Hello";
            char testChars[6] = "Hello";

            // we need the /0 as well
            size_t len = std::strlen(value) + 1;
            size_t size_t_size = sizeof(size_t);

            EXPECT_TRUE(TypeSerializer<char *>::Serialize(*wBuffer, value));
            EXPECT_EQ(len, *((size_t *)(wBuffer->GetData())));
            EXPECT_STREQ(testChars, (wBuffer->GetData() + size_t_size));

            std::unique_ptr<ReadBuffer> rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            std::memset(value, 0, len);
            EXPECT_TRUE(TypeSerializer<char *>::Serialize(*rBuffer, value));
            EXPECT_STREQ(testChars, value);

            // test const value
            wBuffer = std::make_unique<WriteBuffer>();
            const char *testCChars = "Hello";

            EXPECT_TRUE(TypeSerializer<const char *>::Serialize(*wBuffer, testCChars));
            EXPECT_EQ(len, *((size_t *)(wBuffer->GetData())));
            EXPECT_STREQ(testChars, (wBuffer->GetData() + size_t_size));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(TypeSerializer<const char *>::Serialize(*rBuffer, testCChars));
            // use the non const to see that we can read the const we wrote into it
            std::memset(value, 0, len);
            EXPECT_TRUE(TypeSerializer<char *>::Serialize(*rBuffer, value));
            EXPECT_STREQ(testChars, value);
        }

        TEST(TypeSerializerTest, SerializeCharStringSwapping)
        {
            std::unique_ptr<SwappingWriteBuffer> wBuffer = std::make_unique<SwappingWriteBuffer>();

            char value[6] = "Hello";
            char testChars[6] = "Hello";

            // we need the /0 as well
            size_t len = std::strlen(value) + 1;
            size_t swappedLen = len;
            SwapBytes(swappedLen);
            size_t size_t_size = sizeof(size_t);

            EXPECT_TRUE(TypeSerializer<char *>::Serialize(*wBuffer, value));
            EXPECT_EQ(swappedLen, *((size_t *)(wBuffer->GetData())));
            EXPECT_STREQ(testChars, (wBuffer->GetData() + size_t_size));

            std::unique_ptr<SwappingReadBuffer> rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            std::memset(value, 0, len);
            EXPECT_TRUE(TypeSerializer<char *>::Serialize(*rBuffer, value));
            EXPECT_STREQ(testChars, value);

            // test const value
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            const char *testCChars = "Hello";

            EXPECT_TRUE(TypeSerializer<const char *>::Serialize(*wBuffer, testCChars));
            EXPECT_EQ(swappedLen, *((size_t *)(wBuffer->GetData())));
            EXPECT_STREQ(testChars, (wBuffer->GetData() + size_t_size));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(TypeSerializer<const char *>::Serialize(*rBuffer, testCChars));
            // use the non const to see that we can read the const we wrote into it
            std::memset(value, 0, len);
            EXPECT_TRUE(TypeSerializer<char *>::Serialize(*rBuffer, value));
            EXPECT_STREQ(testChars, value);
        }

        TEST(TypeSerializerTest, SerializeWCharString)
        {
            std::unique_ptr<WriteBuffer> wBuffer = std::make_unique<WriteBuffer>();

            // test signed
            wchar_t value[6] = L"Hello";
            wchar_t testChars[6];

            // we need the /0 as well
            size_t len = std::wcslen(value) + 1;
            size_t size_t_size = sizeof(size_t);

            std::memcpy(testChars, value, len * sizeof(wchar_t));

            EXPECT_TRUE(TypeSerializer<wchar_t *>::Serialize(*wBuffer, value));
            EXPECT_EQ(len, *((size_t *)wBuffer->GetData()));
            EXPECT_STREQ(testChars, (wchar_t *)(wBuffer->GetData() + size_t_size));

            std::unique_ptr<ReadBuffer> rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            std::memset(value, 0, len * sizeof(wchar_t));
            EXPECT_TRUE(TypeSerializer<wchar_t *>::Serialize(*rBuffer, value));
            EXPECT_STREQ(testChars, value);

            // test const value
            wBuffer = std::make_unique<WriteBuffer>();
            const wchar_t *testCChars = L"Hello";

            EXPECT_TRUE(TypeSerializer<const wchar_t *>::Serialize(*wBuffer, testCChars));
            EXPECT_EQ(len, *((size_t *)wBuffer->GetData()));
            EXPECT_STREQ(testChars, (wchar_t *)(wBuffer->GetData() + size_t_size));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(TypeSerializer<const wchar_t *>::Serialize(*rBuffer, testCChars));
            // use the non const to see that we can read the const we wrote into it
            std::memset(value, 0, len * sizeof(wchar_t));
            EXPECT_TRUE(TypeSerializer<wchar_t *>::Serialize(*rBuffer, value));
            EXPECT_STREQ(testChars, value);
        }

        TEST(TypeSerializerTest, SerializeWCharStringSwapping)
        {
            std::unique_ptr<SwappingWriteBuffer> wBuffer = std::make_unique<SwappingWriteBuffer>();

            // test signed
            wchar_t value[6] = L"Hello";
            wchar_t testChars[6];
            wchar_t testSwapped[6];

            // we need the /0 as well
            size_t len = std::wcslen(value) + 1;
            size_t size_t_size = sizeof(size_t);
            size_t swappedLen = len;
            SwapBytes(swappedLen);

            std::memcpy(testChars, value, len * sizeof(wchar_t));
            std::memcpy(testSwapped, value, len * sizeof(wchar_t));
            for (size_t i = 0; i < len; i++)
            {
                SwapBytes(testSwapped[i]);
            }

            EXPECT_TRUE(TypeSerializer<wchar_t *>::Serialize(*wBuffer, value));
            EXPECT_EQ(swappedLen, *((size_t *)wBuffer->GetData()));
            EXPECT_STREQ(testSwapped, (wchar_t *)(wBuffer->GetData() + size_t_size));

            std::unique_ptr<SwappingReadBuffer> rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            std::memset(value, 0, len * sizeof(wchar_t));
            EXPECT_TRUE(TypeSerializer<wchar_t *>::Serialize(*rBuffer, value));
            EXPECT_STREQ(testChars, value);

            // test const value
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            const wchar_t *testCChars = L"Hello";

            EXPECT_TRUE(TypeSerializer<const wchar_t *>::Serialize(*wBuffer, testCChars));
            EXPECT_EQ(swappedLen, *((size_t *)wBuffer->GetData()));
            EXPECT_STREQ(testSwapped, (wchar_t *)(wBuffer->GetData() + size_t_size));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(TypeSerializer<const wchar_t *>::Serialize(*rBuffer, testCChars));
            // use the non const to see that we can read the const we wrote into it
            std::memset(value, 0, len * sizeof(wchar_t));
            EXPECT_TRUE(TypeSerializer<wchar_t *>::Serialize(*rBuffer, value));
            EXPECT_STREQ(testChars, value);
        }

        TEST(TypeSerializerTest, SerializeStdString)
        {
            std::unique_ptr<WriteBuffer> wBuffer = std::make_unique<WriteBuffer>();

            std::string value = "Hello";
            std::string testChars = value;

            // we need the /0 as well
            size_t len = value.size();
            size_t size_t_size = sizeof(size_t);

            EXPECT_TRUE(TypeSerializer<std::string>::Serialize(*wBuffer, value));
            EXPECT_EQ(len, *((size_t *)(wBuffer->GetData())));
            EXPECT_STREQ(testChars.c_str(), (wBuffer->GetData() + size_t_size));

            std::unique_ptr<ReadBuffer> rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value.clear();
            EXPECT_TRUE(TypeSerializer<std::string>::Serialize(*rBuffer, value));
            EXPECT_EQ(testChars, value);

            // test const value
            wBuffer = std::make_unique<WriteBuffer>();
            const std::string testCChars = testChars;

            EXPECT_TRUE(TypeSerializer<const std::string>::Serialize(*wBuffer, testCChars));
            EXPECT_EQ(len, *((size_t *)(wBuffer->GetData())));
            EXPECT_STREQ(testChars.c_str(), (wBuffer->GetData() + size_t_size));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(TypeSerializer<const std::string>::Serialize(*rBuffer, testCChars));
            // use the non const to see that we can read the const we wrote into it
            value.clear();
            EXPECT_TRUE(TypeSerializer<std::string>::Serialize(*rBuffer, value));
            EXPECT_EQ(testChars, value);
        }

        TEST(TypeSerializerTest, SerializeStdStringSwapping)
        {
            std::unique_ptr<SwappingWriteBuffer> wBuffer = std::make_unique<SwappingWriteBuffer>();

            std::string value = "Hello";
            std::string testChars = value;

            // we need the /0 as well
            size_t len = value.size();
            size_t size_t_size = sizeof(size_t);
            size_t swappedLen = len;
            SwapBytes(swappedLen);

            EXPECT_TRUE(TypeSerializer<std::string>::Serialize(*wBuffer, value));
            EXPECT_EQ(swappedLen, *((size_t *)(wBuffer->GetData())));
            EXPECT_STREQ(testChars.c_str(), (wBuffer->GetData() + size_t_size));

            std::unique_ptr<SwappingReadBuffer> rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value.clear();
            EXPECT_TRUE(TypeSerializer<std::string>::Serialize(*rBuffer, value));
            EXPECT_EQ(testChars, value);

            // test const value
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            const std::string testCChars = testChars;

            EXPECT_TRUE(TypeSerializer<const std::string>::Serialize(*wBuffer, testCChars));
            EXPECT_EQ(swappedLen, *((size_t *)(wBuffer->GetData())));
            EXPECT_STREQ(testChars.c_str(), (wBuffer->GetData() + size_t_size));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(TypeSerializer<const std::string>::Serialize(*rBuffer, testCChars));
            // use the non const to see that we can read the const we wrote into it
            value.clear();
            EXPECT_TRUE(TypeSerializer<std::string>::Serialize(*rBuffer, value));
            EXPECT_EQ(testChars, value);
        }

        TEST(TypeSerializerTest, SerializeStdWString)
        {
            std::unique_ptr<WriteBuffer> wBuffer = std::make_unique<WriteBuffer>();

            // test signed
            std::wstring value = L"Hello";
            std::wstring testChars = value;

            // we need the /0 as well
            size_t len = value.size();
            size_t size_t_size = sizeof(size_t);

            EXPECT_TRUE(TypeSerializer<std::wstring>::Serialize(*wBuffer, value));
            EXPECT_EQ(len, *((size_t *)wBuffer->GetData()));
            EXPECT_STREQ(testChars.c_str(), (wchar_t *)(wBuffer->GetData() + size_t_size));

            std::unique_ptr<ReadBuffer> rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value.clear();
            EXPECT_TRUE(TypeSerializer<std::wstring>::Serialize(*rBuffer, value));
            EXPECT_EQ(testChars, value);

            // test const value
            wBuffer = std::make_unique<WriteBuffer>();
            const std::wstring testCChars = testChars;

            EXPECT_TRUE(TypeSerializer<const std::wstring>::Serialize(*wBuffer, testCChars));
            EXPECT_EQ(len, *((size_t *)wBuffer->GetData()));
            EXPECT_STREQ(testChars.c_str(), (wchar_t *)(wBuffer->GetData() + size_t_size));

            rBuffer = std::make_unique<ReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(TypeSerializer<const std::wstring>::Serialize(*rBuffer, testCChars));
            // use the non const to see that we can read the const we wrote into it
            value.clear();
            EXPECT_TRUE(TypeSerializer<std::wstring>::Serialize(*rBuffer, value));
            EXPECT_EQ(testChars, value);
        }

        TEST(TypeSerializerTest, SerializeStdWStringSwapping)
        {
            std::unique_ptr<SwappingWriteBuffer> wBuffer = std::make_unique<SwappingWriteBuffer>();

            // test signed
            std::wstring value = L"Hello";
            std::wstring testChars = value;
            std::wstring testSwapped = value;

            // we need the /0 as well
            size_t len = value.size();
            size_t size_t_size = sizeof(size_t);
            size_t swappedLen = len;
            SwapBytes(swappedLen);

            for (size_t i = 0; i < len; i++)
            {
                SwapBytes(testSwapped[i]);
            }

            EXPECT_TRUE(TypeSerializer<std::wstring>::Serialize(*wBuffer, value));
            EXPECT_EQ(swappedLen, *((size_t *)wBuffer->GetData()));
            EXPECT_STREQ(testSwapped.c_str(), (wchar_t *)(wBuffer->GetData() + size_t_size));

            std::unique_ptr<SwappingReadBuffer> rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            value.clear();
            EXPECT_TRUE(TypeSerializer<std::wstring>::Serialize(*rBuffer, value));
            EXPECT_EQ(testChars, value);

            // test const value
            wBuffer = std::make_unique<SwappingWriteBuffer>();
            const std::wstring testCChars = testChars;

            EXPECT_TRUE(TypeSerializer<const std::wstring>::Serialize(*wBuffer, testCChars));
            EXPECT_EQ(swappedLen, *((size_t *)wBuffer->GetData()));
            EXPECT_STREQ(testSwapped.c_str(), (wchar_t *)(wBuffer->GetData() + size_t_size));

            rBuffer = std::make_unique<SwappingReadBuffer>(wBuffer->GetData(), wBuffer->BufferSize());

            EXPECT_FALSE(TypeSerializer<const std::wstring>::Serialize(*rBuffer, testCChars));
            // use the non const to see that we can read the const we wrote into it
            value.clear();
            EXPECT_TRUE(TypeSerializer<std::wstring>::Serialize(*rBuffer, value));
            EXPECT_EQ(testChars, value);
        }

        TEST(TypeSerializerTest, StreamOperator)
        {
            WriteBuffer wBuffer;

            std::string testStr = "Hello";
            std::wstring testWStr = L"Hello";

            wBuffer << 5;
            wBuffer << short{10};
            wBuffer << char{100};
            wBuffer << float{200};
            wBuffer << double{300};
            wBuffer << "Hello";
            wBuffer << L"Hello";
            wBuffer << testStr;
            wBuffer << testWStr;

            ReadBuffer rBuffer(wBuffer.GetData(), wBuffer.BufferSize());

            int testInt = 0;
            short testShort = 0;
            char testChar = 0;
            float testFloat = 0;
            double testDouble = 0;
            char testCString[6];
            wchar_t testWString[6];
            std::string testStdStr;
            std::wstring testStdWStr;

            rBuffer >> testInt;
            rBuffer >> testShort;
            rBuffer >> testChar;
            rBuffer >> testFloat;
            rBuffer >> testDouble;
            rBuffer >> testCString;
            rBuffer >> testWString;
            rBuffer >> testStdStr;
            rBuffer >> testStdWStr;

            EXPECT_EQ(5, testInt);
            EXPECT_EQ(10, testShort);
            EXPECT_EQ(100, testChar);
            EXPECT_EQ(200, testFloat);
            EXPECT_EQ(300, testDouble);
            EXPECT_STREQ("Hello", testCString);
            EXPECT_STREQ(L"Hello", testWString);
            EXPECT_EQ(testStdStr, testStr);
            EXPECT_EQ(testStdWStr, testWStr);
        }
    }
}

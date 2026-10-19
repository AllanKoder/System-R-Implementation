#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <bit>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "storage/serialization/row_codec.hpp"

namespace {
    // Build a minimal schema for tests that focus on one column type.
    RowCodec single_column_codec(ColumnType type, bool nullable = false)
    {
        return RowCodec{TableSchema{{ColumnDefinition{"value", type, nullable}}}};
    }

    // Keep expected byte sequences readable without repeating std::byte casts.
    std::vector<std::byte> make_bytes(std::initializer_list<unsigned int> values)
    {
        std::vector<std::byte> bytes;
        for (const auto value : values)
        {
            bytes.push_back(static_cast<std::byte>(value));
        }
        return bytes;
    }
}

TEST_CASE("RowCodec preserves column order with mixed values and NULLs", "[row_codec][roundtrip][nulls]")
{
    // Mix fixed-size and variable-size fields, including a NULL before other values.
    const RowCodec codec{TableSchema{{
        ColumnDefinition{"id", ColumnType::INT, false},
        ColumnDefinition{"name", ColumnType::VARCHAR, false},
        ColumnDefinition{"rating", ColumnType::DOUBLE, true},
        ColumnDefinition{"active", ColumnType::BIT, false},
        ColumnDefinition{"code", ColumnType::CHAR, false},
        ColumnDefinition{"balance", ColumnType::BIGINT, false},
        ColumnDefinition{"note", ColumnType::VARCHAR, true}
    }}};
    const RowValues row{std::int32_t{42}, std::string{"Alice"}, std::monostate{},
        false, std::string{"AB"}, std::int64_t{-500}, std::monostate{}};

    const auto encoded = codec.Encode(row);
    const auto decoded = codec.Decode(encoded);

    // Only columns 2 and 6 are NULL; false is still a non-null boolean value.
    REQUIRE(encoded.at(0) == std::byte{0x44}); // 0b1000100
    REQUIRE(decoded == row);
}

TEST_CASE("RowCodec encodes and decodes exact integer wire bytes", "[row_codec][integers][wire]")
{
    const RowCodec codec{TableSchema{{
        ColumnDefinition{"tiny", ColumnType::TINYINT, false},
        ColumnDefinition{"small", ColumnType::SMALLINT, false},
        ColumnDefinition{"int", ColumnType::INT, false},
        ColumnDefinition{"big", ColumnType::BIGINT, false}
    }}};
    const RowValues row{std::int8_t{-1}, std::int16_t{-2},
        std::int32_t{0x12345678}, std::int64_t{0x0102030405060708}};
    // A zero null bitmap is followed by signed integer bits, most-significant byte first.
    const auto expected = make_bytes({0x00, 0xFF, 0xFF, 0xFE,
        0x12, 0x34, 0x56, 0x78, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});

    // Check each direction independently so matching encoder/decoder bugs cannot cancel out.
    REQUIRE(codec.Encode(row) == expected);
    REQUIRE(codec.Decode(expected) == row);
}

TEMPLATE_TEST_CASE("RowCodec preserves signed integer boundaries", "[row_codec][integers]",
    std::int8_t, std::int16_t, std::int32_t, std::int64_t)
{
    constexpr ColumnType type = sizeof(TestType) == 1 ? ColumnType::TINYINT
        : sizeof(TestType) == 2 ? ColumnType::SMALLINT
        : sizeof(TestType) == 4 ? ColumnType::INT : ColumnType::BIGINT;
    const auto codec = single_column_codec(type);

    for (const auto value : {std::numeric_limits<TestType>::min(), TestType{-1},
        TestType{0}, TestType{1}, std::numeric_limits<TestType>::max()})
    {
        // Include the failing input in diagnostics; cast int64_t so it prints as a number.
        CAPTURE(static_cast<std::int64_t>(value));
        const RowValues row{value};
        REQUIRE(codec.Decode(codec.Encode(row)) == row);
    }
}

TEST_CASE("RowCodec stores booleans as single bytes", "[row_codec][wire]")
{
    const auto codec = single_column_codec(ColumnType::BIT);
    for (const bool value : {false, true})
    {
        CAPTURE(value);
        const auto expected = make_bytes({0x00, value ? 0x01u : 0x00u});
        REQUIRE(codec.Encode(RowValues{value}) == expected);
        REQUIRE(codec.Decode(expected) == RowValues{value});
    }
}

TEST_CASE("RowCodec uses IEEE big-endian floating-point wire bytes", "[row_codec][float][wire]")
{
    const auto float_codec = single_column_codec(ColumnType::FLOAT);
    const auto double_codec = single_column_codec(ColumnType::DOUBLE);
    // After the null bitmap, these are the IEEE 754 bits for 1.0f and -1.0.
    const auto float_bytes = make_bytes({0x00, 0x3F, 0x80, 0x00, 0x00});
    const auto double_bytes = make_bytes({0x00, 0xBF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    REQUIRE(float_codec.Encode(RowValues{1.0f}) == float_bytes);
    REQUIRE(float_codec.Decode(float_bytes) == RowValues{1.0f});
    REQUIRE(double_codec.Encode(RowValues{-1.0}) == double_bytes);
    REQUIRE(double_codec.Decode(double_bytes) == RowValues{-1.0});
}

TEMPLATE_TEST_CASE("RowCodec preserves floating-point values and signed zero", "[row_codec][float]",
    float, double)
{
    const auto codec = single_column_codec(sizeof(TestType) == 4 ? ColumnType::FLOAT : ColumnType::DOUBLE);
    for (const auto value : {TestType{0.0}, TestType{-0.0}, TestType{-1.25},
        std::numeric_limits<TestType>::min(), std::numeric_limits<TestType>::max(),
        std::numeric_limits<TestType>::lowest(), std::numeric_limits<TestType>::denorm_min(),
        std::numeric_limits<TestType>::infinity(), -std::numeric_limits<TestType>::infinity()})
    {
        CAPTURE(value);
        const auto decoded = codec.Decode(codec.Encode(RowValues{value}));
        const auto actual = std::get<TestType>(decoded.at(0));
        REQUIRE(actual == value);
        // Equality alone cannot distinguish positive zero from negative zero.
        REQUIRE(std::signbit(actual) == std::signbit(value));
    }
}

TEST_CASE("RowCodec canonicalizes NaN payloads", "[row_codec][float][wire]")
{
    const auto float_codec = single_column_codec(ColumnType::FLOAT);
    const auto double_codec = single_column_codec(ColumnType::DOUBLE);
    const auto float_bytes = make_bytes({0x00, 0x7F, 0xC0, 0x00, 0x00});
    const auto double_bytes = make_bytes({0x00, 0x7F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    // Negative NaNs with custom payloads must encode as the standard positive quiet NaNs.
    REQUIRE(float_codec.Encode(RowValues{std::bit_cast<float>(std::uint32_t{0xFFC01234})}) == float_bytes);
    REQUIRE(double_codec.Encode(RowValues{std::bit_cast<double>(std::uint64_t{0xFFF8000000001234ull})}) == double_bytes);
    REQUIRE(std::isnan(std::get<float>(float_codec.Decode(float_bytes).at(0))));
    REQUIRE(std::isnan(std::get<double>(double_codec.Decode(double_bytes).at(0))));
}

TEST_CASE("RowCodec prefixes strings with a two-byte length", "[row_codec][strings][wire]")
{
    for (const auto type : {ColumnType::CHAR, ColumnType::VARCHAR})
    {
        CAPTURE(static_cast<int>(type));
        const auto codec = single_column_codec(type);
        // Explicit length preserves the embedded null and the non-ASCII byte.
        const RowValues row{std::string{"A\0\xFF", 3}};
        // Null bitmap, two-byte length, then the three unmodified string bytes.
        const auto expected = make_bytes({0x00, 0x00, 0x03, 0x41, 0x00, 0xFF});
        REQUIRE(codec.Encode(row) == expected);
        REQUIRE(codec.Decode(expected) == row);
        REQUIRE(codec.Encode(RowValues{std::string{}}) == make_bytes({0x00, 0x00, 0x00}));
        REQUIRE(codec.Decode(make_bytes({0x00, 0x00, 0x00})) == RowValues{std::string{}});
    }
}

TEST_CASE("RowCodec enforces string length boundaries", "[row_codec][strings]")
{
    for (const auto type : {ColumnType::CHAR, ColumnType::VARCHAR})
    {
        const auto codec = single_column_codec(type);
        const RowValues row{std::string(65535, 'x')};
        const auto encoded = codec.Encode(row);
        // A uint16_t length allows 65535 bytes, plus one bitmap byte and two length bytes.
        REQUIRE(encoded.size() == 65538);
        REQUIRE(encoded.at(1) == std::byte{0xFF});
        REQUIRE(encoded.at(2) == std::byte{0xFF});
        REQUIRE(codec.Decode(encoded) == row);
        REQUIRE_THROWS_AS(codec.Encode(RowValues{std::string(65536, 'x')}), std::invalid_argument);
    }
}

TEST_CASE("RowCodec null bitmap spans byte boundaries", "[row_codec][nulls][wire]")
{
    std::vector<ColumnDefinition> columns;
    RowValues row(10, std::monostate{});
    for (std::size_t index = 0; index < row.size(); ++index)
    {
        columns.emplace_back(std::to_string(index), ColumnType::TINYINT, true);
    }
    row[1] = std::int8_t{42};
    row[8] = std::int8_t{-1};
    const RowCodec codec{TableSchema{columns}};
    // A set bit means NULL; only columns 1 and 8 contribute payload bytes.
    const auto expected = make_bytes({0xFD, 0x02, 0x2A, 0xFF});

    REQUIRE(codec.Encode(row) == expected);
    REQUIRE(codec.Decode(expected) == row);
}

TEST_CASE("RowCodec handles a maximum-width all-null row", "[row_codec][nulls]")
{
    std::vector<ColumnDefinition> columns;
    for (std::size_t index = 0; index < 255; ++index)
    {
        columns.emplace_back(std::to_string(index), ColumnType::INT, true);
    }
    const RowCodec codec{TableSchema{columns}};
    const RowValues row(255, std::monostate{});
    std::vector<std::byte> expected(32, std::byte{0xFF});
    // The last byte describes seven columns; its unused highest bit stays clear.
    expected.back() = std::byte{0x7F};

    REQUIRE(codec.Encode(row) == expected);
    REQUIRE(codec.Decode(expected) == row);
}

TEST_CASE("RowCodec distinguishes null strings from empty strings", "[row_codec][nulls][strings]")
{
    const auto codec = single_column_codec(ColumnType::VARCHAR, true);
    // NULL has no payload; an empty string still has a two-byte zero length.
    REQUIRE(codec.Encode(RowValues{std::monostate{}}) == make_bytes({0x01}));
    REQUIRE(codec.Encode(RowValues{std::string{}}) == make_bytes({0x00, 0x00, 0x00}));
    REQUIRE(codec.Decode(make_bytes({0x01})) == RowValues{std::monostate{}});
    REQUIRE(codec.Decode(make_bytes({0x00, 0x00, 0x00})) == RowValues{std::string{}});
}

TEST_CASE("RowCodec rejects null in non-nullable columns", "[row_codec][invalid]")
{
    const auto codec = single_column_codec(ColumnType::INT);
    REQUIRE_THROWS_AS(codec.Encode(RowValues{std::monostate{}}), std::invalid_argument);
    REQUIRE_THROWS_AS(codec.Decode(make_bytes({0x01})), std::invalid_argument);
}

TEST_CASE("RowCodec rejects mismatched value types", "[row_codec][invalid]")
{
    for (const auto type : {ColumnType::BIT, ColumnType::TINYINT, ColumnType::SMALLINT,
        ColumnType::INT, ColumnType::BIGINT, ColumnType::FLOAT, ColumnType::DOUBLE})
    {
        CAPTURE(static_cast<int>(type));
        const auto codec = single_column_codec(type);
        REQUIRE_THROWS_AS(codec.Encode(RowValues{std::string{"wrong type"}}), std::invalid_argument);
    }
    for (const auto type : {ColumnType::CHAR, ColumnType::VARCHAR})
    {
        const auto codec = single_column_codec(type);
        REQUIRE_THROWS_AS(codec.Encode(RowValues{std::int32_t{1}}), std::invalid_argument);
    }
}

TEST_CASE("RowCodec handles mixed rows and rejects every truncated prefix", "[row_codec][invalid][roundtrip]")
{
    const RowCodec codec{TableSchema{{
        ColumnDefinition{"bool", ColumnType::BIT, false},
        ColumnDefinition{"tiny", ColumnType::TINYINT, false},
        ColumnDefinition{"small", ColumnType::SMALLINT, false},
        ColumnDefinition{"int", ColumnType::INT, false},
        ColumnDefinition{"big", ColumnType::BIGINT, false},
        ColumnDefinition{"float", ColumnType::FLOAT, false},
        ColumnDefinition{"double", ColumnType::DOUBLE, false},
        ColumnDefinition{"char", ColumnType::CHAR, false},
        ColumnDefinition{"varchar", ColumnType::VARCHAR, false}
    }}};
    const RowValues row{true, std::int8_t{-1}, std::int16_t{-2}, std::int32_t{-3},
        std::int64_t{-4}, 1.25f, -2.5, std::string{"abc"}, std::string{"def"}};
    const auto encoded = codec.Encode(row);
    REQUIRE(codec.Decode(encoded) == row);
    // Cut the row at every byte boundary to exercise truncated headers and field payloads.
    for (std::size_t length = 0; length < encoded.size(); ++length)
    {
        CAPTURE(length);
        const std::vector<std::byte> prefix(encoded.begin(), encoded.begin() + length);
        REQUIRE_THROWS_AS(codec.Decode(prefix), std::invalid_argument);
    }
    // A complete row must also reject unexpected bytes after its last field.
    auto trailing = encoded;
    trailing.push_back(std::byte{0});
    REQUIRE_THROWS_AS(codec.Decode(trailing), std::invalid_argument);
}

TEST_CASE("RowCodec rejects string lengths exceeding the available payload", "[row_codec][invalid][strings]")
{
    for (const auto type : {ColumnType::CHAR, ColumnType::VARCHAR})
    {
        const auto codec = single_column_codec(type);
        // The prefix claims 65535 string bytes, but only one payload byte follows.
        REQUIRE_THROWS_AS(codec.Decode(make_bytes({0x00, 0xFF, 0xFF, 0x41})), std::invalid_argument);
    }
}

TEST_CASE("RowCodec handles an empty schema", "[row_codec][empty]")
{
    const RowCodec codec{TableSchema{{}}};
    // With no columns there is no bitmap or payload, so even one byte is extra.
    REQUIRE(codec.Encode({}).empty());
    REQUIRE(codec.Decode({}).empty());
    REQUIRE_THROWS_AS(codec.Decode(make_bytes({0x00})), std::invalid_argument);
}

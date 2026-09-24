#pragma once

// https://www.w3schools.com/sql/sql_datatypes.asp
enum class ColumnType
{
    BIT, // bool
    TINYINT, // int8_t
    SMALLINT, // int16_t
    INT, // int32_t
    BIGINT, // int64_t
    FLOAT, // float
    DOUBLE, // double
    CHAR, // string
    VARCHAR // string
};

#include <iostream>
#include <iomanip>
#include <cstddef>

#include "row_codec.hpp"

int main()
{
    ColumnDefinition column1("first", ColumnType::BIT, true);
    ColumnDefinition column2("second", ColumnType::BIT, true);
    ColumnDefinition column3("third", ColumnType::BIT, true);
    TableSchema schema({column1, column2, column3});

    RowCodec row_codec(schema);
    const auto encoded = row_codec.Encode({true, {}, false});
    std::cout << "Encoded bytes: " << encoded.size() << "\n";
    std::cout << "Hex: " << std::hex << std::setfill('0');
    for (const auto byte : encoded)
    {
        std::cout << std::setw(2) << std::to_integer<unsigned int>(byte) << ' ';
    }
    std::cout << std::dec << "\n";

}

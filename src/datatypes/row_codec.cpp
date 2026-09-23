#include <assert.h>

#include "row_codec.hpp"

RowCodec::RowCodec(TableSchema schema): schema_{schema} {};

std::string RowCodec::Encode(const RowValues& row_values) const
{
    std::vector<ColumnDefinition> columns = schema_.GetAllColumns();

    assert(columns.size() == row_values.size() && "RowCodec::Encode requires row_values.size() to match columns.size()");

    std::string
    for(int i = 0; i < columns.size(); i++)
    {
        ColumnType& column_type = columns[i].type;
        switch (column_type)
        {
        case (ColumnType::BIT):

            break;

        default:
            break;
        }
    }
}

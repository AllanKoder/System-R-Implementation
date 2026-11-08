#include "storage/serialization/row_codec.hpp"

class RowSchema
{
public:
    RowValueKey GetPrimaryKey() const;
    char* GetData(); // Excludes key
    char* GetCompleteData() const; // Includes the key


}

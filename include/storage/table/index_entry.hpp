#pragma once

#include "storage/serialization/row_codec.hpp"

class IndexEntry
{
public:
    IndexEntry(RowValueKey key, RowValueKey id);

    const RowValueKey& GetKey() const noexcept;
    const RowValueKey& GetId() const noexcept;

private:
    RowValueKey key_;
    RowValueKey id_;
};

#include <optional>
#include <storage/serialization/row_codec.hpp>

class RowIterator {
public:
    virtual ~RowIterator() = default;

    virtual void Open(std::optional<RowValueKey> low = std::nullopt, std::optional<RowValueKey> high = std::nullopt) = 0;
    virtual std::optional<RowValue> GetNext() = 0;
    virtual void Close() noexcept = 0;
};

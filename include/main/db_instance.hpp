#include <string>
#include "lmdb.h"
// Class which holds the entire database instance, it will be passed around in order to actually interact with the API's that we want.
class DbInstance final {
public:
    // define the virtual memory file upper limit.
    // 1UL << 30 bytes = 1 GB
    explicit DbInstance(std::string path, size_t database_size_bytes = (1UL << 30));

    MDB_env* GetEnv();
    // Get Config
    // Get Execution Env
    // Get
private:
    MDB_env *env_;
};

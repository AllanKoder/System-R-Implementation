#include "main/db_instance.hpp"
#include "lmdb/lmdb_utils.h"

DbInstance::DbInstance(std::string path, size_t database_size_bytes):
row_codec_()
{
    LMDB_CHECK(mdb_env_create(&env_));
    LMDB_CHECK(mdb_env_set_mapsize(env_, database_size_bytes));
    // default flags are fine
    // Permissions: (Owner can read + write, Group can read+write, Other can only read)
    LMDB_CHECK(mdb_env_open(env_, path.data(), 0, 0664));
}

MDB_env* DbInstance::GetEnv()
{
    return env_;
}

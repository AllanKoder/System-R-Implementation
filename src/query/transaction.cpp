
#include "lmdb/transaction.hpp"
#include "lmdb/lmdb_utils.h"

Txn::Txn(MDB_env *env, std::optional<std::reference_wrapper<Txn>> parent, unsigned int flags) {
    if (parent.has_value())
    {
        LMDB_CHECK(mdb_txn_begin(env, parent->get().Get(), flags, &txn_));
        return;
    }
    LMDB_CHECK(mdb_txn_begin(env, NULL, flags, &txn_));
}

Txn::~Txn() {
    if (txn_) {
        mdb_txn_abort(txn_);
    }
}

void Txn::Commit() {
    MDB_txn *txn = txn_;
    txn_ = nullptr;
    LMDB_CHECK(mdb_txn_commit(txn));
}

MDB_txn *Txn::Get() const {
    return txn_;
}

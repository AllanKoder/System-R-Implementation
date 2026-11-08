#pragma once

#include "lmdb.h"

class Txn {
public:
	explicit Txn(MDB_env *env, unsigned int flags = 0);
	~Txn();

	void Commit();
	MDB_txn *Get() const;

    Txn(const Txn& other) = delete;
    Txn& operator=(const Txn& other) = delete;

private:
	MDB_txn *txn_ = nullptr;
};

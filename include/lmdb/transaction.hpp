#pragma once
#include <optional>
#include <functional>
#include "lmdb.h"

class Txn {
public:
	explicit Txn(MDB_env *env, std::optional<std::reference_wrapper<Txn>> parent = std::nullopt, unsigned int flags = 0);
	~Txn();

	void Commit();
	MDB_txn *Get() const;

    Txn(const Txn& other) = delete;
    Txn& operator=(const Txn& other) = delete;

private:
	MDB_txn *txn_ = nullptr;
};

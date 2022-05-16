#pragma once

#include "../connection_container.h"

namespace sqlite_orm {
    namespace internal {
        inline void storage_base::register_migration(int from, int to, migration_t migration) {
            migration_key key{from, to};
            this->migrations[key] = move(migration);
        }

        inline void storage_base::migrate_to(int to) {
            auto con = this->get_connection();  //  we must keep the connection
            auto currentVersion = this->pragma.user_version();
            migration_key key{currentVersion, to};
            auto it = this->migrations.find(key);
            if(it != this->migrations.end()) {
                auto& migration = it->second;
                connection_container connectionContainer(this->connection);
                migration(connectionContainer);
            } else {
                throw std::system_error{orm_error_code::migration_not_found};
            }
        }
    }
}

#pragma once

#include <memory>  //  std::shared_ptr

#include "connection_holder.h"
#include "storage.h"

namespace sqlite_orm {
    namespace internal {
        struct storage_base;
    }

    struct connection_container {

        template<class... Ts>
        internal::storage_t<Ts...> make_storage(Ts... tables) const {
            using ReturnType = internal::storage_t<Ts...>;
            return ReturnType(this->connection_holder, std::forward<Ts>(tables)...);
        }

        connection_container(std::shared_ptr<internal::connection_holder> connection_holder) :
            connection_holder(move(connection_holder)) {}

      private:
        friend struct internal::storage_base;

        std::shared_ptr<internal::connection_holder> connection_holder;
    };
}

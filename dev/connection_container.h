#pragma once

#include <memory>  //  std::shared_ptr

#include "connection_holder.h"
#include "storage.h"

namespace sqlite_orm {
    struct connection_container {

        /*template<class... Ts>
        internal::storage_t<Ts...> make_storage(Ts... tables) const {
            
        }*/
        connection_container(std::shared_ptr<internal::connection_holder> connection_holder) :
            connection_holder(move(connection_holder)) {}

      private:
        std::shared_ptr<internal::connection_holder> connection_holder;
    };
}

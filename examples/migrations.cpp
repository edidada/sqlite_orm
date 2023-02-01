#include <sqlite_orm/sqlite_orm.h>
#include <iostream>

using namespace sqlite_orm;
using std::cout;
using std::endl;

/**
 *  Migrations logic is based on PRAGMA user_version which is by default equal 0.
 *  This example shows how migrations API helps avoiding dropping data during `sync_schema` call
 *  and altering data for new schema instead.
 */
int main() {
    auto filename = "migrations.sqlite";
    ::remove(filename);

    //  perform the first version
    {
        //  this is a model which will be updated later
        struct User {
            int id = 0;
            std::string name;
        };

        auto storage = make_storage(
            filename,
            make_table("users", make_column("id", &User::id, primary_key()), make_column("name", &User::name)));
        storage.sync_schema();

        //  insert data
        storage.replace(User{1, "Sertab Erener"});
        storage.replace(User{2, "Inna"});
    }

    /**
     *  Now let's assume we need to alter the schema: we decided to remove `name` column and create `first_name` and `last_name` columns instead.
     *  Changing schema and calling `sync_schema` will drop all rows in `users` table cause storage doesn't know new values for two new columns.
     *  But we know it: we want to split every existing name by space character and set the first part as `first_name` and the rest as `second_name`.
     *  So `Sertab Erener` will become `Sertab` and `Erener`, but `Inna` will become `Inna` and `` (empty string). Assume it is ok for us. This is
     *  a logic of our migration from user_version 0 to user_version 1.
     */
    {
        struct User {
            int id = 0;
            std::string firstName;
            std::string lastName;
        };
        auto storage = make_storage(filename,
                                    make_table("users",
                                               make_column("id", &User::id, primary_key()),
                                               make_column("first_name", &User::firstName),
                                               make_column("last_name", &User::lastName)));

        /**
         *  This is a place where we specify our migration logic for 0 -> 1 migration. Function `register_migration` must be called once to
         *  register a migration. It accepts 3 arguments: from user_version, to user_version and a callback with 1 argument: const ref to
         *  `connection_container` instance. `connection_container` is a new class designed especially for migrations API and is intended to be
         *  used within migration callback only. This callback will be called once we try to migrate to user_version 1 and current user_version is 0.
         *  This callback is responsible to preserve important data, call `sync_schema` and insert updated data again.
         *  Of course migration may be create without `sync_schema` call and without schema changes at all (e.g. we decided to add 'Mr' as a prefix
         *  to all names). But this is a case with schema changing.
         */
        storage.register_migration(0, 1, [&storage](const connection_container& connection) {
            /**
             *  `OldUser` is a replica from `User` from the very first code version.
             */
            struct OldUser {
                int id = 0;
                std::string name;
            };
            /**
             *  Use `connection` to make an old storage. Actually `oldStorage` and `storage` will have the same `sqlite3*` handler
             *  but they will have different sqlite_orm schema. `oldStorage` will have old schema and allows working with the
             *  storage with old schema. `oldStorage` is useful before `sync_schema` call.
             */
            auto oldStorage = connection.make_storage(make_table("users",
                                                                 make_column("id", &OldUser::id, primary_key()),
                                                                 make_column("name", &OldUser::name)));

            //  get all old users (which have `name` column)
            auto oldUsers = oldStorage.get_all<OldUser>();
            storage.sync_schema();  //  sync schema. Data will be dropped here

            //  insert data again with modifications we want
            for(auto& oldUser: oldUsers) {
                User newUser;
                newUser.id = oldUser.id;
                auto spaceIndex = oldUser.name.find(' ');
                if(spaceIndex != oldUser.name.npos) {
                    newUser.firstName = oldUser.name.substr(0, spaceIndex);
                    newUser.lastName = oldUser.name.substr(spaceIndex + 1, oldUser.name.length() - spaceIndex - 1);
                } else {
                    newUser.firstName = oldUser.name;
                }
                storage.replace(newUser);
            }
        });

        //  this call makes a migration if current user_version is different or does nothing if current
        //  user version is 1
        storage.migrate_to(1);

        //  print all suers to make sure we did good
        auto allUsers = storage.get_all<User>();
        cout << "users: " << allUsers.size() << endl;
        for(auto& user: allUsers) {
            cout << storage.dump(user) << endl;
        }
    }

    return 0;
}

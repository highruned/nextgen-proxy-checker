#ifndef NEXTGEN_SOCIAL
#define NEXTGEN_SOCIAL

#include "common.h"


namespace nextgen
{
    namespace social
    {
        struct basic_country_variables
        {
           basic_country_variables() : code("null"), id(0)
            {

            }

            std::string code;
            uint32_t id;
        };

        class basic_country
        {
            NEXTGEN_ATTACH_SHARED_VARIABLES(basic_country, basic_country_variables);
        };

        struct basic_gender_variables
        {
            basic_gender_variables() : code("null"), id(0)
            {

            }

            std::string code;
            uint32_t id;
        };

        class basic_gender
        {
            NEXTGEN_ATTACH_SHARED_VARIABLES(basic_gender, basic_gender_variables);
        };

        struct basic_name_variables
        {
            basic_name_variables() : first("null"), middle("null"), last("null")
            {

            }

            std::string first;
            std::string middle;
            std::string last;
        };

        class basic_name
        {
            NEXTGEN_ATTACH_SHARED_VARIABLES(basic_name, basic_name_variables);
        };

        struct basic_email_variables
        {
            basic_email_variables(nextgen::network::smtp_server server) : server(server), user("null"), host("null")
            {

            }

            nextgen::network::smtp_server server;
            std::string user;
            std::string host;
        };

        class basic_email
        {
            public: typedef std::function<void(std::string)> receive_successful_event_type;

            public: void receive(receive_successful_event_type successful_handler) const
            {
                auto self = *this;

                self->server->handler_list[self.to_string()] += successful_handler;
            }

            public: std::string to_string() const
            {
                auto self = *this;

                return self->user + "@" + self->host;
            }

            NEXTGEN_ATTACH_SHARED_VARIABLES(basic_email, basic_email_variables);
        };

        struct basic_person_variables
        {
            basic_person_variables() : id(0), postal_code("null")
            {

            }

            uint32_t id;
            basic_name name;
            boost::gregorian::date birthday;
            std::string postal_code;
            basic_country country;
            basic_gender gender;
        };

        class basic_person
        {
            NEXTGEN_ATTACH_SHARED_VARIABLES(basic_person, basic_person_variables);
        };

        struct basic_user_variables
        {
            basic_user_variables() : username("null"), password("null"), email(null)
            {

            }

            std::string username;
            std::string password;
            basic_email email;
        };

        class basic_user
        {
            NEXTGEN_ATTACH_SHARED_VARIABLES(basic_user, basic_user_variables);
        };

        struct basic_account_variables
        {
            basic_account_variables() : id(0), type(0), user(null), person(null)
            {

            }

            uint32_t id;
            uint32_t type;
            basic_user user;
            basic_person person;
        };

        template<typename variables_type>
        class basic_account
        {
            public: struct types
            {
                static const uint32_t none = 0;
                static const uint32_t google = 1;
                static const uint32_t youtube = 2;
            };

            NEXTGEN_ATTACH_SHARED_VARIABLES(basic_account, variables_type);
        };

        struct basic_service_variables
        {
            basic_service_variables(database::link main_database) : main_database(main_database)
            {

            }

            event<std::function<void(void)>> person_list_empty_event;

            std::vector<basic_person> person_list;
            database::link main_database;
        };

        class basic_service
        {
            public: void update()
            {
                auto self = *this;

                if(self->person_list.size() == 0)
                {
                    self->person_list_empty_event();

                }
            }

            public: void add_person(basic_person person)
            {
                auto self = *this;

                self->person_list.push_back(person);
            }

            public: basic_person get_random_person()
            {
                auto self = *this;

                return self->person_list[random(0, (int)self->person_list.size()-1)];
            }

            NEXTGEN_ATTACH_SHARED_VARIABLES(basic_service, basic_service_variables);
        };

        typedef basic_person person;
        typedef basic_account<basic_account_variables> account;
        typedef basic_email email;
        typedef basic_name name;
        typedef basic_country country;
        typedef basic_gender gender;
        typedef basic_user user;

        typedef basic_service service;
    }
}

#endif

#ifndef NEXTGEN_SOCIAL
#define NEXTGEN_SOCIAL

#include "common.h"


namespace nextgen
{
    namespace social
    {
        namespace detail
        {
            class country_variables
            {
                public: country_variables() : code("null"), id(0)
                {

                }

                std::string code;
                uint32_t id;
            };

            class basic_country
            {
                NEXTGEN_ATTACH_SHARED_VARIABLES(basic_country, country_variables);
            };

            class gender_variables
            {
                public: gender_variables() : code("null"), id(0)
                {

                }

                std::string code;
                uint32_t id;
            };

            class basic_gender
            {
                NEXTGEN_ATTACH_SHARED_VARIABLES(basic_gender, gender_variables);
            };

            class name_variables
            {
                public: name_variables() : first("null"), middle("null"), last("null")
                {

                }

                std::string first;
                std::string middle;
                std::string last;
            };

            class basic_name
            {
                NEXTGEN_ATTACH_SHARED_VARIABLES(basic_name, name_variables);
            };

            class email_variables
            {
                public: email_variables(nextgen::network::smtp_server server) : server(server), user("null"), host("null")
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

                NEXTGEN_ATTACH_SHARED_VARIABLES(basic_email, email_variables);
            };

            class person_variables
            {
                public: person_variables() : id(0), postal_code("null")
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
                NEXTGEN_ATTACH_SHARED_VARIABLES(basic_person, person_variables);
            };

            class account_variables
            {
                public: account_variables() : id(0), type(0), username("null"), password("null"), email("null")
                {

                }

                uint32_t id;
                uint32_t type;
                std::string username;
                std::string password;
                std::string email;
                basic_person person;
            };

            class basic_account
            {
                public: struct types
                {
                    static const uint32_t none = 0;
                    static const uint32_t google = 1;
                    static const uint32_t youtube = 2;
                };

                NEXTGEN_ATTACH_SHARED_VARIABLES(basic_account, account_variables);
            };

            class service_variables
            {
                public: service_variables()
                {

                }
            };

            class basic_service
            {
                public: basic_person get_random_person()
                {
                    return basic_person();
                }

                NEXTGEN_ATTACH_SHARED_VARIABLES(basic_service, service_variables);
            };
        }

        typedef detail::basic_person person;
        typedef detail::basic_account account;
        typedef detail::basic_email email;
        typedef detail::basic_name name;
        typedef detail::basic_country country;
        typedef detail::basic_gender gender;

        typedef detail::basic_service service;
    }
}

#endif

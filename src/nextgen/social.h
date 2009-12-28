#ifndef NEXTGEN_SOCIAL
#define NEXTGEN_SOCIAL

#include "common.h"


namespace nextgen
{
    namespace social
    {
        class country
        {
            NEXTGEN_SHARED_CLASS(country, NEXTGEN_SHARED_CLASS_VARS(
            {
                variables() : code("null"), id(0)
                {

                }

                std::string code;
                uint32_t id;
            }));
        };

        class gender
        {
            NEXTGEN_SHARED_CLASS(gender, NEXTGEN_SHARED_CLASS_VARS(
            {
                variables() : code("null"), id(0)
                {

                }

                std::string code;
                uint32_t id;
            }));
        };

        class name
        {
            NEXTGEN_SHARED_CLASS(name, NEXTGEN_SHARED_CLASS_VARS(
            {
                variables() : first("null"), middle("null"), last("null")
                {

                }

                std::string first;
                std::string middle;
                std::string last;
            }));
        };

        class person
        {
            NEXTGEN_SHARED_CLASS(person, NEXTGEN_SHARED_CLASS_VARS(
            {
                variables() : id(0), postal_code("null")
                {

                }

                nextgen::social::name name;
                boost::gregorian::date birthday;
                std::string postal_code;
                nextgen::social::country country;
                nextgen::social::gender gender;
                uint32_t id;
            }));
        };

        class email
        {
            public: typedef std::function<void()> receive_successful_event_type;

            public: void receive(receive_successful_event_type successful_handler) const
            {
                //self->server->handler_list[self->user + "@" + self->host] = successful_handler;
            }

            public: operator std::string() const
            {
                auto self = *this;

                return self->user + "@" + self->host;
            }

            private: NEXTGEN_SHARED_CLASS(email, NEXTGEN_SHARED_CLASS_VARS(
            {
                variables(nextgen::network::smtp_server server) : server(server), user("null"), host("null")
                {

                }

                std::string user;
                std::string host;
                nextgen::network::smtp_server server;
            }));
        };

        class account
        {
            NEXTGEN_SHARED_CLASS(account, NEXTGEN_SHARED_CLASS_VARS(
            {
                variables() : id(0), type(0), username("null"), password("null"), email("null")
                {

                }

                uint32_t id;
                std::string username;
                std::string password;
                uint32_t type;
                nextgen::social::person person;
                std::string email;
            }));

            public: struct types
            {
                static const uint32_t none = 0;
                static const uint32_t google = 1;
                static const uint32_t youtube = 2;
            };
        };
    }
}

#endif

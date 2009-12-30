#ifndef NEXTGEN_DATABASE
#define NEXTGEN_DATABASE

#include "common.h"

namespace nextgen
{
    namespace database
    {
        typedef boost::shared_ptr<boost::unordered_map<std::string, std::string>> row;
        typedef boost::shared_ptr<std::vector<row>> row_list;

        class link
        {
            public: typedef row row_type;
            public: typedef row_list row_list_type;

            public: void connect(std::string const& host, std::string const& username, std::string const& password, std::string const& database = "") const
            {
                auto self = *this;

                if(self->connected != null_str && self->connected != database)
                    self.disconnect();

                self->link = mysql_init(NULL);

                if(!self->link)
                {
                    std::cout << "[nextgen:database:link] MySQL init error" << std::endl;

                    return;
                }

                // Connect to MySQL.
                if(mysql_real_connect(self->link, host.c_str(), username.c_str(), password.c_str(), database.c_str(), 0, NULL, 0) == NULL)
                {
                    std::cout << "<Database> Error connecting to " << database.c_str() << "." << std::endl;

                    std::cout << "<MySQL> " << mysql_error(NULL) << std::endl;

                    mysql_close(self->link);
                    mysql_library_end();

                    return;
                }

                self->connected = database;
            }

            public: void disconnect() const
            {
                auto self = *this;

                mysql_close(self->link);

                self->link = (MYSQL*)NULL;

                self->connected = null_str;
            }


            public: void query(std::string const& query) const
            {
                auto self = *this;

                if(self->connected == null_str)
                {
                    std::cout << "<Database> Not connected." << std::endl;

                    return;
                }

                MYSQL_RES *result;
                MYSQL_ROW row;

                int status = mysql_query(self->link, query.c_str());

                if(status)
                {
                    printf("Could not execute statement(s)");
                    printf(mysql_error(self->link));
                    mysql_close(self->link);
                }

                MYSQL_FIELD *field;
                unsigned int num_fields;
                unsigned int i;
                unsigned long *lengths;

                std::vector<std::string> fields;

                /* process each statement result */
                do
                {
                    /* did current statement return data? */
                    result = mysql_store_result(self->link);

                    if(result)
                    {
                        /* yes; process rows and free the result set */
                        //process_result_set(mysql, result);

                        num_fields = mysql_num_fields(result);

                        for(i = 0; i < num_fields; i++)
                        {
                            field = mysql_fetch_field(result);
                            fields.push_back(field->name);
                        }

                        while((row = mysql_fetch_row(result)))
                        {
                           lengths = mysql_fetch_lengths(result);

                           //NextGen::Framework::Database::Row hash;

                           for(i = 0; i < num_fields; i++)
                           {
                               //hash[fields[i]] = row[i];
                               //std::cout << fields[i] << ": " << row[i] << std::endl;
                           }
                        }

                        mysql_free_result(result);
                    }
                    else          /* no result set or error */
                    {
                        if(mysql_field_count(self->link) == 0)
                        {

                            //printf("%lld rows affected\n",
                            //mysql_affected_rows(this->link));
                        }
                        else  /* some error occurred */
                        {
                            printf("Could not retrieve result set\n");

                            break;
                        }
                    }

                    /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
                    if((status = mysql_next_result(self->link)) > 0)
                        printf("Could not execute statement\n");

                } while (status == 0);

                //boost::this_thread::sleep(boost::posix_time::milliseconds(15));
            }


            public: row_list_type get_row_list(std::string const& query) const
            {
                auto self = *this;

                row_list_type list(new row_list_type::element_type);

                if(self->connected == null_str)
                {
                    std::cout << "<Database> Not connected." << std::endl;

                    return list;
                }

                MYSQL_RES* result;
                MYSQL_ROW row;
                //std::cout << "a" << std::endl;
                int status = mysql_query(self->link, query.c_str());
                //std::cout << "1" << std::endl;
                if(status)
                {
                    printf("Could not execute statement(s)");
                    printf(mysql_error(self->link));
                    mysql_close(self->link);
                    return list;
                }
            //std::cout << "2" << std::endl;
                MYSQL_FIELD *field;
                unsigned int num_fields;
                unsigned int i;
                unsigned long *lengths;

                std::vector<std::string> fields;

                do
                {
                    result = mysql_store_result(self->link);
            //std::cout << "3" << std::endl;
                    if(result)
                    {
                        //process_result_set(mysql, result);

                        num_fields = mysql_num_fields(result);

                        for(i = 0; i < num_fields; i++)
                        {
                            field = mysql_fetch_field(result);
                            fields.push_back(field->name);
                        }

                        while((row = mysql_fetch_row(result)))
                        {
                           lengths = mysql_fetch_lengths(result);

                           row_type hash(new row_type::element_type);

                           for(i = 0; i < num_fields; i++)
                           {
                               if(row[i])
                               {
                                    (*hash.get())[fields[i]] = row[i];
                                    //std::cout << fields[i] << ": " << row[i] << std::endl;
                               }
                               else
                               {
                                    (*hash.get())[fields[i]] = "NULL";
                                    //std::cout << fields[i] << ": " << "NULL" << std::endl;
                               }
                           }
            //std::cout << "4" << std::endl;
                           list->push_back(hash);
                        }

                        mysql_free_result(result);
                    }
                    else
                    {
                        //std::cout << "5" << std::endl;
                        if(mysql_field_count(self->link) == 0)
                        {
                            printf("%lld rows affected\n",
                            mysql_affected_rows(self->link));
                        }
                        else
                        {
                            printf("Could not retrieve result set\n");

                            break;
                        }
                    }

                    if((status = mysql_next_result(self->link)) > 0)
                        printf("Could not execute statement\n");
            //std::cout << "6" << std::endl;
                } while (status == 0);

                return list;
            }


            public: row_type get_row(std::string const& query) const
            {
                auto self = *this;

                row_type hash(new row_type::element_type);

                if(self->connected == null_str)
                {
                    std::cout << "<Database> Not connected." << std::endl;

                    return hash;
                }

                MYSQL_RES* result;
                MYSQL_ROW row;
                //std::cout << "a" << std::endl;
                int status = mysql_query(self->link, query.c_str());
                //std::cout << "1" << std::endl;
                if(status)
                {
                    printf("Could not execute statement(s)");
                    printf(mysql_error(self->link));
                    mysql_close(self->link);
                    return hash;
                }
            //std::cout << "2" << std::endl;
                MYSQL_FIELD *field;
                unsigned int num_fields;
                unsigned int i;
                unsigned long *lengths;

                std::vector<std::string> fields;

                do
                {
                    result = mysql_store_result(self->link);
            //std::cout << "3" << std::endl;
                    if(result)
                    {
                        //process_result_set(mysql, result);

                        num_fields = mysql_num_fields(result);

                        for(i = 0; i < num_fields; i++)
                        {
                            field = mysql_fetch_field(result);
                            fields.push_back(field->name);
                        }

                        while((row = mysql_fetch_row(result)))
                        {
                           lengths = mysql_fetch_lengths(result);

                           for(i = 0; i < num_fields; i++)
                           {
                               if(row[i])
                               {
                                    (*hash.get())[fields[i]] = row[i];
                                    //std::cout << fields[i] << ": " << row[i] << std::endl;
                               }
                               else
                               {
                                    (*hash.get())[fields[i]] = "NULL";
                                    //std::cout << fields[i] << ": " << "NULL" << std::endl;
                               }
                           }
            //std::cout << "4" << std::endl;

                        }

                        mysql_free_result(result);
                    }
                    else
                    {
                        //std::cout << "5" << std::endl;
                        if(mysql_field_count(self->link) == 0)
                        {
                            printf("%lld rows affected\n",
                            mysql_affected_rows(self->link));
                        }
                        else
                        {
                            printf("Could not retrieve result set\n");

                            break;
                        }
                    }

                    if((status = mysql_next_result(self->link)) > 0)
                        printf("Could not execute statement\n");
            //std::cout << "6" << std::endl;
                } while (status == 0);

                return hash;
            }

            private: struct variables
            {
                variables() : connected(null_str)
                {

                }

                MYSQL* link;
                std::string connected;
            };

            NEXTGEN_SHARED_DATA(link, variables);
        };
    }
}

#endif

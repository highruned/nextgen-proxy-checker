#ifndef NEXTGEN_MATH
#define NEXTGEN_MATH

#include "common.h"

namespace nextgen
{
    namespace math
    {
        /**
        Old vector class I had laying around. Will do for now. Added NEXTGEN_SHARED_DATA. Removed std::vector.
        */
        template<typename element_type>
        class vector;

        template<typename element_type> vector<element_type> operator+(vector<element_type> const&, vector<element_type> const&);
        template<typename element_type> vector<element_type> operator-(vector<element_type> const&, vector<element_type> const&);
        template<typename element_type> vector<element_type> operator*(vector<element_type> const&, float);
        template<typename element_type> vector<element_type> operator*(float, vector<element_type> const&);
        template<typename element_type> vector<element_type> operator/(vector<element_type> const&, float);
        template<typename element_type> vector<element_type> operator/(float, vector<element_type> const&);
        template<typename element_type> vector<element_type> operator--(vector<element_type> const&);
        template<typename element_type> vector<element_type> operator++(vector<element_type> const&);

        template <typename element_type>
        class vector
        {
            //public: virtual const string to_string() const;

            public: static vector<element_type> null() { return vector<element_type>(0, 0, 0, 0); }

            public: static vector<element_type> up() { return vector<element_type>(0, 1, 0, 0); }
            public: static vector<element_type> down() { return vector<element_type>(0, -1, 0, 0); }

            public: static vector<element_type> left() { return vector<element_type>(-1, 0, 0, 0); }
            public: static vector<element_type> right() { return vector<element_type>(1, 0, 0, 0); }

            public: static vector<element_type> unit_x() { return vector<element_type>(1, 0, 0, 0); }
            public: static vector<element_type> unit_y() { return vector<element_type>(0, 1, 0, 0); }
            public: static vector<element_type> unit_z() { return vector<element_type>(0, 0, 1, 0); }

            public: static vector<element_type> zero() { return vector<element_type>(0, 0, 0, 0); }
            public: static vector<element_type> one() { return vector<element_type>(1, 1, 1, 1); }

            public: vector<element_type> operator=(vector<element_type> v)
            {
                auto self = *this;

                self->x_ = v->x_;
                self->y_ = v->y_;
                self->z_ = v->z_;
                self->w_ = v->w_;

                return self;
            }

            public: element_type x() { auto self = *this; return self->x_; }
            public: element_type y() { auto self = *this; return self->y_; }
            public: element_type z() { auto self = *this; return self->z_; }
            public: element_type w() { auto self = *this; return self->w_; }

            public: void x(element_type x_) { auto self = *this; if(x_ != self->x_) { self->x_ = x_; self->changed = true; } }
            public: void y(element_type y_) { auto self = *this; if(y_ != self->y_) { self->y_ = y_; self->changed = true; } }
            public: void z(element_type z_) { auto self = *this; if(z_ != self->z_) { self->z_ = z_; self->changed = true; } }
            public: void w(element_type w_) { auto self = *this; if(w_ != self->w_) { self->w_ = w_; self->changed = true; } }

            public: bool is_changed() { auto self = *this; return self->changed; }

            //private: friend std::ostream& operator<<(std::ostream&, const vector<element_type>&);
            private: template<typename y> friend vector<y> operator+(vector<y>, vector<y>);
            private: template<typename y> friend vector<y> operator-(vector<y>, vector<element_type>);
            private: template<typename y> friend vector<y> operator*(vector<y>, float);
            private: template<typename y> friend vector<y> operator*(float, vector<y>);
            private: template<typename y> friend vector<y> operator/(vector<y>, float);
            private: template<typename y> friend vector<y> operator/(float, vector<y>);
            private: template<typename y> friend vector<y> operator--(vector<y>);
            private: template<typename y> friend vector<y> operator++(vector<y>);

            private: struct variables
            {
                variables(element_type const x_ = 0, element_type const y_ = 0, element_type const z_ = 0, element_type const w_ = 0) : changed(true)
                {
                    this->x_ = x_;
                    this->y_ = y_;
                    this->z_ = z_;
                    this->w_ = w_;
                }

                ~variables()
                {

                }

                element_type x_;
                element_type y_;
                element_type z_;
                element_type w_;
                bool changed;
            };

            NEXTGEN_SHARED_DATA(vector, variables);
        };

        //template<class element_type> using vector2 = vector<element_type>;
        //template<class element_type> using vector3 = vector<element_type>;
/*
        template <typename element_type>
        class vector2 : public vector<element_type>
        {
            //public: explicit vector2() : vector<element_type>() { }

            //public: operator vector3();
            //public: operator vector3() const;
            //public: operator vector4();
            //public: operator vector4() const;
        };*/

        /*
        template <typename element_type>
        inline std::ostream& operator<<(std::ostream& os, const vector<element_type>& v)
        {
            os << v.to_string();

            return os;
        }*/

        template <typename element_type>
        inline vector<element_type> operator+(vector<element_type> v1, vector<element_type> v2)
        {
            return v1.clone().add(v2);
        }

        template <typename element_type>
        inline vector<element_type> operator-(vector<element_type> v1, vector<element_type> v2)
        {
            return v1.clone().subtract(v2);
        }

        template <typename element_type>
        inline vector<element_type> operator*(vector<element_type> v, float m)
        {
            return v.clone().multiply(m);
        }

        template <typename element_type>
        inline vector<element_type> operator*(float m, vector<element_type> v)
        {
            return v.clone().multiply(m);
        }

        template <typename element_type>
        inline vector<element_type> operator/(vector<element_type> v, float m)
        {
            return v.clone().divide(m);
        }

        template <typename element_type>
        inline vector<element_type> operator/(float m, vector<element_type> v)
        {
            return v.clone().divide(m);
        }

        template <typename element_type>
        inline vector<element_type> operator--(vector<element_type> v)
        {
            v->x_ = --v->x_;
            v->y_ = --v->y_;
            v->z_ = --v->z_;
            v->w_ = --v->w_;

            return v;
        }

        template <typename element_type>
        inline vector<element_type> operator++(vector<element_type> v)
        {
            v->x_ = ++v->x_;
            v->y_ = ++v->y_;
            v->z_ = ++v->z_;
            v->w_ = ++v->w_;

            return v;
        }
/*
        template <typename element_type>
        inline vector<element_type> operator=(vector<element_type> const& v1, vector<element_type> const& v2)
        {
            v1->x(v2->x());
            v1->y(v2->y());
            v1->z(v2->z());
            v1->w(v2->w());

            return v1;
        }*/
    }
}

#endif

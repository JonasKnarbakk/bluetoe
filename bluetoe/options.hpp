#ifndef BLUETOE_OPTIONS_HPP
#define BLUETOE_OPTIONS_HPP

#include <utility>
#include <type_traits>
#include <tuple>

namespace bluetoe {
namespace details {

    /*
     * Select A or B by Select. If Select == true, the result is A; B otherwise
     */
    template < bool Select, typename A, typename B >
    struct select_type {
        typedef A type;
    };

    template < typename A, typename B >
    struct select_type< false, A, B > {
        typedef B type;
    };

    /*
     * return A if A is not Null, otherwise return B if B is not Null, otherwise Null
     */
    template < typename Null, typename A, typename B >
    struct or_type {
        typedef A type;
    };

    template < typename Null, typename B >
    struct or_type< Null, Null, B > {
        typedef B type;
    };

    /*
     *  add A to B
     */
    template < class A, class B >
    struct add_type
    {
        typedef std::tuple< A, B > type;
    };

    template < class T, class ... Ts >
    struct add_type< T, std::tuple< Ts... > >
    {
        typedef std::tuple< T, Ts... > type;
    };

    template < class ...Ts, class T >
    struct add_type< std::tuple< Ts... >, T >
    {
        typedef std::tuple< Ts..., T > type;
    };

    template < class ...As, class ...Bs >
    struct add_type< std::tuple< As... >, std::tuple< Bs...> >
    {
        typedef std::tuple< As..., Bs... > type;
    };

    /*
     * a tuple type with two elements
     */
    template < typename A, typename B >
    struct pair {
        typedef A first_type;
        typedef B second_type;
    };

    template < typename T >
    struct not_type
    {
        typedef std::integral_constant< bool, !T::value > type;
    };

    /*
     * empty result
     */
    struct no_such_type {};

    template < typename T >
    struct extract_meta_type
    {
        template < class U >
        static typename U::meta_type check( U* )  { return U::meta_type(); }
        static no_such_type          check( ... ) { return no_such_type(); }

        typedef decltype( check( static_cast< T* >( nullptr ) ) ) meta_type;
        typedef meta_type                                         type;
        typedef typename not_type<
            typename std::is_same<
                meta_type, no_such_type >::type >::type  has_meta_type;
    };

    /*
     * finds the first type that has the embedded meta_type
     */
    template <
        typename MetaType,
        typename ... Types >
    struct find_by_meta_type;

    template <
        typename MetaType >
    struct find_by_meta_type< MetaType >
    {
        typedef no_such_type type;
    };

    template <
        typename MetaType,
        typename Type >
    struct find_by_meta_type< MetaType, Type >
    {
        typedef extract_meta_type< Type > meta_type;
        typedef typename select_type<
            std::is_same< typename meta_type::type, MetaType >::value,
            Type, no_such_type >::type type;
    };

    template <
        typename MetaType,
        typename Type,
        typename ... Types >
    struct find_by_meta_type< MetaType, Type, Types... >
    {
        typedef typename or_type<
            no_such_type,
            typename find_by_meta_type< MetaType, Type >::type,
            typename find_by_meta_type< MetaType, Types... >::type >::type type;
    };

    /*
     * finds all types that has the embedded meta_type
     */
    template <
        typename MetaType,
        typename ... Types >
    struct find_all_by_meta_type;

    template <
        typename MetaType >
    struct find_all_by_meta_type< MetaType >
    {
        typedef std::tuple<> type;
    };

    template <
        typename MetaType,
        typename Type,
        typename ... Types >
    struct find_all_by_meta_type< MetaType, Type, Types... >
    {
        typedef extract_meta_type< Type > meta_type;
        typedef typename select_type<
            std::is_same< typename meta_type::type, MetaType >::value,
            typename add_type< Type, typename find_all_by_meta_type< MetaType, Types... >::type >::type,
            typename find_all_by_meta_type< MetaType, Types... >::type >::type type;
    };

    /*
     * counts the number of Types with a given MetaType
     */
    template <
        typename MetaType,
        typename ... Types >
    struct count_by_meta_type;

    template <
        typename MetaType >
    struct count_by_meta_type< MetaType > {
        enum { count = 0 };
    };

    template <
        typename MetaType,
        typename Type,
        typename ... Types >
    struct count_by_meta_type< MetaType, Type, Types... > {
        typedef extract_meta_type< Type > meta_type;

        enum { count = std::is_same< typename meta_type::type, MetaType >::value
            ? 1 + count_by_meta_type< MetaType, Types... >::count
            : 0 + count_by_meta_type< MetaType, Types... >::count
        };
    };

    /**
     * @brief returns true, if Option is given in Options
     */
    template <
        typename Option,
        typename ... Options >
    struct has_option;

    template <
        typename Option >
    struct has_option< Option > {
        static bool constexpr value = false;
    };

    template <
        typename Option,
        typename FirstOption,
        typename ... Options >
    struct has_option< Option, FirstOption, Options... > {
        static bool constexpr value =
            std::is_same< Option, FirstOption >::value || has_option< Option, Options... >::value;
    };

    /**
     * @brief executes f.each< O >() for every O in Options.
     *
     * For
     * Example:
     * @code
    struct value_printer
    {
        template< typename O >
        void each()
        {
            std::cout << x << 'n';
        }
    };

    int main()
    {
        for_< Options... >::each( value_printer() );
    }
     * @endcode
     */
    template <
        typename ... Options >
    struct for_;

    template <>
    struct for_<>
    {
        template < typename Function >
        static void each( Function )
        {}
    };

    template <
        typename Option >
    struct for_< Option >
    {
        template < typename Function >
        static void each( Function f )
        {
            f.template each< Option >();
        }
    };

    template <
        typename Option,
        typename ... Options >
    struct for_< Option, Options... >
    {
        template < typename Function >
        static void each( Function f )
        {
            f.template each< Option >();
            for_< Options... >::each( f );
        }
    };


}
}

#endif


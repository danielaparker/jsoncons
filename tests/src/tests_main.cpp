#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <iostream>

#define EXPAND(X) X    


#define NARGS(...) NARG_(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define NARG_(...) EXPAND( ARG_N(__VA_ARGS__) )
#define ARG_N(e1, e2, e3, e4, e5, N, ...) N

#define FOO(Seq) FOO_1( NARGS Seq ) 
#define FOO_1(Count) std::cout << "Sequence count: " << Count <<"\n"; FOO_2 (Count)
#define FOO_2(Count) BAR_ ## Count ()
#define BAR_2() std::cout << "Two members\n\n"
#define BAR_3() std::cout << "Three members\n\n"

TEST_CASE("macro madness")
{
    int a, b, c;
    FOO((a, b));
    FOO((a, b, c));
}


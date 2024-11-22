//#include <SQL>
#include <vector>
#include <variant>

int main(void) {
    std::vector<std::variant<double, int>> vec;

    vec.push_back(3.14);
    vec.push_back(3);


    return 0;
}
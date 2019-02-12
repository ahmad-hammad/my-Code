#ifndef H_FUNC_H
#define H_FUNC_H

#include <vector>
#include <string>
#include <sstream>
using namespace std;
class h_func
{
    public:
        h_func();
        virtual ~h_func();
        static vector<string> split(string str);
        static void upper(string* s);
    protected:
    private:
};

#endif // H_FUNC_H

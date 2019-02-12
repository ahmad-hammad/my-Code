#include "h_func.h"


vector<string> h_func::split(string str)
{

    string buf;
    std::stringstream ss(str);
    vector<string> tokens;

    while (ss >> buf)
        tokens.push_back(buf);

    return tokens;

}

void h_func:: upper(string* s){
    string temp="";
for(int i =0 ; i<s->length();i++){

    temp+=toupper(s->at(i));
}

*s=temp;
}
h_func::h_func()
{
    //ctor
}

h_func::~h_func()
{
    //dtor
}

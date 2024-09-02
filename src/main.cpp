#include <iostream>
#include <thread>
#include <chrono>

#include "../include/Interpreter.h"
#include "../include/LogManager.h"

#include "../include/Compiler.h"

using namespace std;
using namespace BongoJam;

int main()
{
    LogManager::MainLogger().Initialize("../logs", "MainLogger");
    LogManager::MainLogger().Debug(("MainLogger successfully initialized!"), "main");

    string SourceCode = 
        R"(if (20 <= 30 ) "////////////////AWWWWWWWWWWWWWWWWWWWYE" //>w<
                { 
                    print("Hello World!");
                    let x -> public int = 99.69 - 30;
                }  
                while( text _dn != "your_mom" ) 
                { 
                    if( 5 =================================================== 5)
                    {
                        print("Who's in Paris?", "red"); 
                        let f_MyDictionary = new Dictionary[];
                    }
                } 
                #no_frills
                func main
                (
                    args --------> text, 
                    your_mom -> decimal
                ) --------------------------------------> int
                {
                    5 += 2 -= 3 *= /= %= 3;
                    return 0;  
                }
                @DN 
               )";
    5 + 5 + 5 + 5 + 5 + 5 + 5 + 5 + 5 + 5 + 5;
    vector<Token> Tokens = Tokenize(SourceCode);
    
    for (Token _t : Tokens)
    {
        cout << 
            CreateColouredText("token value: ", "bright magenta") << _t.m_Value << 
            CreateColouredText(" token type: ", "bright cyan") << static_cast<int>(_t.m_Type) <<
            CreateColouredText(" line number: ", "bright yellow") << _t.m_SourceCodeLineNumber << 
        "\n";
    }

    BongoInterpreter* Interpreter = new BongoInterpreter();

    vector<uint8_t> HelloWorld = 
    {
        0x019, //print() bytecode instruction
    };

   EncodeUTF8String(HelloWorld, "Hello World!");


    //Interpreter->RunBongoScript(HelloWorld);

    delete Interpreter;
    Interpreter = nullptr;

    int _n = 0;

    int _lim = _n + 5;

    for (_n; _n < _lim; _n++)
    {
        cout << _n << "\n";
    }

    //cout << "Hello World!" << "\n";
    
    return 0;
}
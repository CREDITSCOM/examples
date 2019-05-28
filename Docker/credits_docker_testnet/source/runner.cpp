#include <thread>
#include <iostream>
#include <sstream>

using namespace std;

void run_contract_executor()
{
    system("java -jar contract-executor.jar");
}

void run_node(char* params)
{
    ostringstream ss;
    ss << "./client " << params;
    system(ss.str().c_str());
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cout << "Usage: ./runner \"parameters\"" << endl;
        cout << "For example: ./runner \"--db-path test_db/ --public-key-file test_keys/public.txt --private-key-file test_keys/private.txt\"" << endl;
        return 1;
    }

    thread th1(run_contract_executor);
    thread th2(run_node, ref(argv[1]));

    th1.join();
    th2.join();
}



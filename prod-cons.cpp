#include <vector>
#include "semaphore.h"

using namespace std;

vector<int> arr, result;
int sp = 0;
Semaphore s1(0);

void produce(Semaphore &s, int &sp, vector<int> &arr){
    int temp = rand() % 101;
    arr.push_back(temp);
    result.push_back(temp*temp);
    s.release();
}

void consume(Semaphore &s, int &sp, vector<int> &arr){
    s.acquire();
    cout << arr[sp]*arr[sp] << ", ";
    sp++;
}

int main(int argc, char const *argv[])
{
    vector<thread> t;
    for (int i=0; i<15; i++){
        t.push_back(thread(produce, ref(s1), ref(sp), ref(arr)));
        t.push_back(thread(consume, ref(s1), ref(sp), ref(arr)));
    }

    for (int i=0; i<30; i++){
        t[i].join();
    }
    cout << endl << "Should've been: " << endl;

    for (int i=0; i<15; i++){
        cout << result[i] << ", ";
    }



    return 0;
}

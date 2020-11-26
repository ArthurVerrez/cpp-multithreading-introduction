//#pragma GCC optimize("Ofast")
//#pragma GCC optimize("inline")
//#pragma GCC optimize("omit-frame-pointer")
//#pragma GCC optimize("unroll-loops")

#include <iostream>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

float norm(vector<float> U, int n){
    // Must compute the sum of root of the absolute value of each component.
    float ans = 0;
    for (int i = 0 ; i < n ; i++){
        ans += sqrt(abs(U[i]));
    }
    return ans;
};

int main()
{
    vector<float> vec {1.0, 1.0, 1.0};
    float ans = norm(vec,3);
    cout << ans  << endl;
}
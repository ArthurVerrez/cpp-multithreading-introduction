//#pragma GCC optimize("Ofast")
//#pragma GCC optimize("inline")
//#pragma GCC optimize("omit-frame-pointer")
//#pragma GCC optimize("unroll-loops")

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <time.h>

using namespace std;

class Timer
{
public:
  Timer(const std::string& name)
    : name_ (name),
      start_ (clock())
    {
    }
  ~Timer()
    {
      double elapsed = (double(clock() - start_) / double(CLOCKS_PER_SEC));
      std::cout << name_ << ": " << int(elapsed * 1000) << "ms" << std::endl;
    }
private:
  std::string name_;
  clock_t start_;
}; 

#define TIMER(name) Timer timer__(name);

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
    TIMER("Norm");
    float ans = norm(vec,3);
    cout << ans  << endl;
}
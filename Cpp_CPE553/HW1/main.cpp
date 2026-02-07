#include <iostream>
#include <cmath>

using namespace std; // if not used, must write std::cout or std::endl


//PROBLEM 1 : Sum from 1 to n
static int sumUpTo(int n)
{
    int res = 0;

    for (int i = 1; i <= n; i++) {
        res += i;
    }

    return res;
}

//PROBLEM 2 : Sum from 1 to n of 1/i (normal and reversed)
static float sumReciprocalNormal(float n)
{
    float res = 0;

    for (float i = 1; i <= n; i++)  {
        res += (1 / i);
    }

    return res;
}

static float sumReciprocalRev(float n)
{
    float res = 0;

    for (float i = n; i >= 1; i--)  {
        res += (1 / i);
    }
    return res;
}

/* 
    COMPARE a. and b.

    a. and b. should output the exact same result, the only difference comes from
    the way the loop is designed for each of them, the first define i starting at 1
    and increment each time to n, the other starts at n and decrement each time until 1

*/

//PROBLEM 3 : Sum from 1 to n of 1/i*i (normal and reversed)
static float sumMysteryNormal(float n)
{
    float res = 0;

    for (float i = 1; i <= n; i++) {
        res += (1/(i*i));
    }

    return res;
}

static float sumMysteryRev(float n)
{
    float res = 0;

    for (float i = n; i >= 1; i--) {
        res += (1/(i*i));
    }

    return res;
}


int main() 
{
    // Problem One
    cout << sumUpTo(100) << '\n';


    // Problem Two
    cout << sumReciprocalNormal(100) << '\n';
    cout << sumReciprocalRev(100) << '\n';
    //float sum1;// compute the sum 1/1 + 1/2 + 1/3 + 1/4 + 1/5 + ... + 1/100
    //float sum2;// compute the sum 1/100 + 1/99 + 1/98 + ... + 1/1 = 5.18
    // use function you made
    //cout << sum1 << '\n';
    //cout << sum2 << '\n';
    //cout << (sum1 - sum2) << '\n';
    // inclusive: 1*1 + 2*2 + 3*3 + 4*4 + 5*5
    // inclusive: 1*1 + 2*2 + 3*3 + 4*4 + 5*5 +6*6 + 7*7
    // Problem Three
    cout << sumMysteryNormal(100) << '\n';
    cout << sumMysteryRev(100) << '\n';
    // mystery1sum
    // be careful! if you calculate as int this will not work. use float!
    // compute 1.0/(1*1) + 1.0/(2.0*2.0) + 1/(3*3) + ... + 1/(n*n) n-> infinity
    //float mystery1; //
    // mystery2sum
    // compute 1/(n*n) + 1/((n-1)*(n-1)) + 1/((n-2)*(n-2)) + ...
    //float mystery2; //
    return 0;
}
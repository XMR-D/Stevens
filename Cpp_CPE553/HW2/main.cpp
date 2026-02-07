#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <string>

#include <iomanip>

//#include "functionWind.h"
//#include "functionPrime.h"

using namespace std;

// const for pi
const double pi = 3.14159265358979323846;

/* Define a Pos type that will be used to store the computed position */
template <typename T1, typename T2>
struct Pos {
    T1 x;
    T2 y;
};

//======================= PROBLEM 1 :

/* IsPrime: Method using trial divide algorithm to check primality */
static bool 
isPrime(int n)
{
    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
           return false;
        }
    }
    return true;
}


//========================= PROBLEM 2
static double 
AngleInRadian(double angle)
{
    return angle * (pi/180);
}

static Pos<double, double> 
ComputePos(Pos<double, double> start_pos, double r, double theta)
{
    Pos<double, double> new_pos;

    new_pos.x = start_pos.x + (r * cos(theta));
    new_pos.y = start_pos.y + (r * sin(theta));

    return new_pos;
}

static Pos<double, double> 
PrintData(Pos<double, double> init_pos, double r, double angle)
{
    cout << "Distance (in meters) / Angle (in degrees): " << fixed << setprecision(8) << r;
    cout << " / " << angle << endl;

    double theta = AngleInRadian(angle);
    Pos<double, double> new_pos = ComputePos(init_pos, r, theta);

    cout << "Angle (in radians): " << theta << endl;
    cout << "New robot position X/Y: ";
    cout << new_pos.x << " / " << new_pos.y << endl;

    return new_pos;
}

static void 
ComputeLocations(const char * filepath) 
{
    ifstream datafile (filepath);
    
    if (datafile.is_open()) {

        cout << "Robotpath data file open successfully" << endl;

        string str_meter;
        string str_angle;
        Pos<double, double> start_pos;
        int i = 1;

        start_pos.x = 0;
        start_pos.y = 0;

        cout << "Assuming Robot start at position (x = 0 / y = 0)" << endl << endl;
        

        while (!datafile.eof()) {

            getline(datafile, str_meter, ' ');
            getline(datafile, str_angle, '\n');

            cout << "step " << i << ' ';
            cout << "_____________" << endl << endl;
            start_pos = PrintData(start_pos, stod(str_meter), stod(str_angle));
            i++;
            cout << endl;
        }

        cout << endl;

        datafile.close();

    } else {
        cout << "Error : Can't open the file with following path : ";
        cout << filepath << endl;
    }
}

//========================= PROBLEM 3
/*
    Write code that Calculate the wind chill using the below formula:
    Wind Chill = 35.74 + 0.6215T – 35.75(V^0.16) + 0.4275T(V^0.16)
    Note: Windchill Temperature is only defined for temperatures at or below 50 degrees F and wind speeds above 3 mph.
    Hint: design a function using the above formula
*/
static double 
Compute_Windchill(double T, double V)
{
    return 35.74 + 0.6215*T - 35.75*pow(V,0.16) + 0.4275*T*pow(V, 0.16);
}

int main() {

    cout << "###########" << endl;
    cout << "Problem One" << endl;
    cout << "###########" << endl;
    // For problem one, you only need to write your code in "functionPrime.cpp"
    // define isPrime

    /* PROBLEM 1 : ANSWER */
    cout << isPrime(19);
    cout << '\n';
    cout << isPrime(81); // trial divide, 81%3 == 0 therefore not prime
    cout << '\n';
    cout << isPrime(57) ; // 57 % 2, 57% 3, 57%4
    cout << '\n';

    cout << "====[ end ]====" << endl << endl;
    cout << "###########" << endl;
    cout << "Problem Two" << endl;
    cout << "###########" << endl << endl;
    // For problem Two, you will need to write your code here
    // first read file robotpath.dat and name it thefile by using ifstream
    // make sure you use the full path to .dat file, e.g., .././src/robotpath.dat
    //check if thefile exist or not
    
    // define your variables each line r and theta from the polar coordinates
    // use defined constant pi value in your calculation
    // start reading file and do calculation
    // while reading print the following for each line:
    // - print distance and angle
    // - print angle in radian, 1 degree is equal (pi/180degree)
    // - print x and y location
    // - check your conversion and print angle in rad after finding x and y with
    //cartesian coordinates conversion

    // don't forget to close your opened file

    /* PROBLEM 2 : ANSWER */
    ComputeLocations("robotpath.dat");
    
    cout << "====[ end ]====" << endl;
    cout << "###########" << endl;
    cout << "Problem Three" << endl;
    cout << "###########" << endl;

    /* PROBLEM 3 : ANSWER */


    //Write your code here
    // use Boolean CanRun to decide to do calculation or not based on condition
    bool canRun = true;
    // define your input value and other variable
    // print out message to enter wind speed and take the input
    // Check if speed meet condition
    // print out message to enter temperature and take the input
    // check if temperature meet condition
    // check if all condition is met and use canRun to start calculation of wind
    // chill
    // print out the value
    double T;
    double V;

    cout << "Enter Temperature : ";
    cin >> T;
    if (T > 50) {
        cout << "Temperature too high to meet computation criteria (criteria : T <= 50)" << endl;
        canRun = false;
    }

    cout << "Enter Wind Speed : ";
    cin >> V;
    if (V <= 3) {
        cout << "Wind to slow to meet computation criteria (criteria : V > 3)" << endl;
        canRun = false;
    }

    if (canRun) {
        cout << Compute_Windchill(T, V) << endl;
    }

    cout << "====[ end ]====" << endl;
    return 0;
}

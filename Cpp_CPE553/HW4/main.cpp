#include <time.h>
#include <iostream>

using namespace std;

//TODO: Ask teacher about importance of mins and seconds and how to handle it


// complete the JulianDate class
// write your code in the JulianDate class
// complete the code in main
class JulianDate {
    // Make all your class access is public
    // first define your variable type and variable (year, month, day, hour, mins, sec)
    // write your non-default constructor that set variables today's date
    
    // write your code here

    /* The object attributes are not accessible outside the class */
    private:
        int _year;
        int _month;
        int _day;
        int _hour;
        int _mins;
        int _sec;

    public:

        /* Julian date object non-default constructor */
        JulianDate(int year, int month, int day, int hour, int mins, int sec) {
            _year = year;
            _month = month;
            _day = day;
            _hour = hour;
            _mins = mins;
            _sec = sec;
        }

        /* Julian date object default constructor */
        JulianDate() {
            _year = 0;
            _month = 0;
            _day = 0;
            _hour = 0;
            _mins = 0;
            _sec = 0; 
        }

        // write function that calculate julian number from given date
        // your function should return the value
        // use calc_juliandate as the function name
        // write your code here

        int calc_juliandate(void) {


            int a = (14 - _month) / 12;
            int m = _month + (12 * a) - 3;
            int y = _year + 4800 - a;

            int inter_y = 365*y + (y/4) - (y/100) + (y/400) - 32045;
            int res = _day + (((153*m)+2)/5) + inter_y;

            return res;

        }
        
        // write function that calculate difference between two Julian date
        // use diff_calc_juliandate as the function name
        // write your code here

        int diff_calc_juliandate(JulianDate b) {
            return calc_juliandate() - b.calc_juliandate();
        }

};

/*
    Use the formula in the below web page in the calculate julian function
    http://www.c-jump.com/bcc/c155c/Homework/a4_Serial_Julian_Date/
    a4_Serial_Julian_Date.html
    https://www.aavso.org/jd-calculator
    hh:mm:ss
    00:00:00 0.0
    12:00:00 0.5
    18*365+18/4 - 18/100 + 18/400
    18*364+4 = 6574
    6574.041666666 --> Jan 1, 2018, 01:00:00
*/

//int JulianDate::EPOCH = 2000; // Jan.1 2000, 00:00:00 = 0

int main() 
{
    cout << "########" << endl;
    cout << "Main Problem" << endl;
    cout << "########" << endl;

    // Run your class on this code and calculate the difference of Julian number below
    cout.precision(20);

    // how many days since day 0? dy = (2018 - 2000) * 365)
    JulianDate x(2007, 2, 8, 0, 0, 0);

    double x1 = x.calc_juliandate();
    cout << x1 << endl;
    JulianDate y(2000, 12, 31, 0, 0, 1);
    
    double y1 = y.calc_juliandate();
    cout << y1 << endl;
    //manual calculation of the difference between two dates, e.g., x1 - y1 = results
    //print out the result
    // write your code here

    cout << "(Manual) Date difference x1 - y1: " << x1 - y1 << endl;
    
    
    // Now use your class function to find the difference between two dates
    // print out the result
    // write your code here
    
    cout << "(Via class) Date difference: " << x.diff_calc_juliandate(y) << endl;
    
    //example of date to Julian date format without hour/min/sec
    //7/4/1776 2369916
    //12/31/2000 2451910
    //2/8/2007 2454140
    //2/9/2007 2454141
    
    
    cout << "====[ end ]====" << endl;
    return 0;
}

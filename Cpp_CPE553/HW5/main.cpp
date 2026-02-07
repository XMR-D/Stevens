#include <iostream>
#include <unordered_map>
#include <string>
#include <vector> // use C++ STL vector with our struct
#include <cmath>
#include <random> // use this to generate random number
#include <fstream>

#include <locale>

using namespace std;


// Define constant double here:
// 1. for G = universal gravitational constant equal 6.674E-11 or 6.674*10^(-11)
// 2. for pi = 3.14159265358979323846
// write your code here

double G = 6.674 * pow(10, -11);
double pi = 3.14159265358979323846;

/* Forward declaration tp solve the Solarsystem interdependance */
class SolarSystem;
class Body;

/* 
    CelestialSpecs : struct that old every specs extracted from .dat file
                    It will be used in BodyCelestialProperties to compute
                    various data. 
    */
struct CelestialSpecs {
    string str_name;
    string str_orbit;
    string str_mass;
    string str_diam;
    string str_perihelion;
    string str_aphelion;
    string str_orbperiod;
    string str_rotationalperiod;
    string str_axialtilt;
    string str_orbinclin;
};

/* Generic function to generate an angle*/
double GenerateAngle(double low_bound, double high_bound)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(low_bound, high_bound);
    return dis(gen);
}

/* 
    FIRST BIG MODIFICATION == THE VEC3D STRUCT HAS BEEN MODIFIED AND TURNED INTO A CLASS 
    THE CLASS IS CALLED BODYPROPERTIES AND DESCRIBE THE PROPERTIES OF A BODY
*/
class BodyCelestialProperties {

    private:
        int x;
        int y;
        int z;

        double velocity_x;
        double velocity_y;
        double velocity_z;

        double acceleration_x;
        double acceleration_y;
        double acceleration_z;

        struct CelestialSpecs spec;
        
        /* define the object primary mass for this body */
        double primary_body_mass;

    public:
        /* Default constructor -- Should not be used */
        BodyCelestialProperties() {
            x = 0;
            y = 0;
            z = 0;
            velocity_x = 0;
            velocity_y = 0;
            velocity_z = 0;
            acceleration_x = 0;
            acceleration_y = 0;
            acceleration_z = 0;
        }

        /* Default constructor */
        BodyCelestialProperties(struct CelestialSpecs spec_) {

            /* rand moduo 11 to generate a number between 0 and 10 inclusive */
            x = rand() % 11;
            y = rand() % 11;
            z = rand() % 11;

            spec = spec_;
        }

        /* BONUS: Constructor with given starting position */
        BodyCelestialProperties(int x_, int y_, int z_, struct CelestialSpecs spec_) {

            x = x_;
            y = y_;
            z = z_;

            spec = spec_;
        }

        void IncreaseAccel(int acc) {
            acceleration_x += acc;
            acceleration_y += acc;
            acceleration_z += acc;
        }

        void IncreaseVel(int vel) {
            velocity_x += vel;
            velocity_y += vel;
            velocity_z += vel;
        }

        /* Setter for primary body mass of the actual object */
        void SetPrimaryBodyMass(double primary_body_mass_) {
            primary_body_mass = primary_body_mass_;
        }

        /* Setter for motions metrics */
        void SetBodyMotion(double ang)
        {
            // approximation of radius of your orbit = (Perihelion  + Aphelion) /2
            double r = (stod(spec.str_perihelion) + stod(spec.str_aphelion))/ 2;
            
            // 	v = sqrt( G"universal gravitational" * mass of the sun /radius )
            double v = sqrt((G * primary_body_mass) / r);

            // a = v^2 / r
            double a = pow(v, 2) / r;

            double scale = 1.0e-9;

            /* 
                If my orbit is NaN it means that Im not influenced
                by another object hence, Im the core of the Solar system
            */
            if (spec.str_orbit != "NaN") {

                /* -v to get velocity as a tangential vector */
                velocity_x = r * cos(ang) * v * scale;
                velocity_y = r * sin(ang) * -v * scale;
                velocity_z = 0;

                /* -a to get acceleration towards center */
                acceleration_x = r * cos(ang) * -a * scale;
                acceleration_y = r * sin(ang) * -a * scale;
                acceleration_z = 0;   
            }
        }

        friend ostream& operator<<(ostream& os, const BodyCelestialProperties &properties);
        friend class Body;
};


// overload << operator for struct AstroVec3d
// when use << so it prints like this: x_value, y_value, z_value
// write your code here
ostream& operator<<(ostream& os, const BodyCelestialProperties &properties) 
{
    os << "Body position  (x, y, z) : ";
    os << properties.x << ", ";
    os << properties.y << ", ";
    os << properties.z << endl;

    os << "Body orbital velocity (m/s) (x, y, z) : ";
    os << properties.velocity_x << ", ";
    os << properties.velocity_y << ", ";
    os << properties.velocity_z << endl;

    os << "Body acceleration (m/s²) (x, y, z) : ";
    os << properties.acceleration_x << ", ";
    os << properties.acceleration_y << ", ";
    os << properties.acceleration_z << endl;

    os << "Primary body associated mass :" << properties.primary_body_mass << endl;
    return os;
}

// ----- Implement class body here ----
// ------------------------------------
// Body class will have private variables to save following information:
//  planet name: string
//  orbit name: string
//  the planet mass: double
//  position  using AstroVec3d: AstroVec3d
//  velocity  using AstroVec3d: AstroVec3d
//  acceleration using AstroVec3d: AstroVec3d

class Body {

    private:
        string name;
        string orbit;
        double mass;
        BodyCelestialProperties properties;

    public:

        Body() {
            name = "none";
            orbit = "none";
            mass = 0;
        }
        
        //non-default constructor to initialize body information.
        // name same as default constructor
        // name, orbit, mass, pos (AstroVec3d overload), v (AstroVec3d overload), a(AstroVec3d overload)
        // write your code here
        Body(string name_, string orbit_, double mass_, BodyCelestialProperties properties_) {
            name = name_;
            orbit = orbit_;
            mass = mass_;
            properties = properties_;
        }

        string GetName(void)
        {
            return name;
        }

        string GetOrbit(void)
        {
            return orbit;
        }

        double GetMass(void) 
        {
            return mass;
        }


    friend ostream& operator<<(ostream& os, const Body &body);
    friend class SolarSystem;

    // create stepAcceleration() function that change the acceleration of planet in SolarSystem
    // this function should loop through Solarsystem object and change acceleration of each body planet
    // the input is vector of Body and integer acceleration
    // for function type use it as: static auto stepAcceleration()
    // write your code here

    void stepAcceleration(int acc) {
        properties.IncreaseAccel(acc);
    }

    void stepVelocity(int acc) {
        properties.IncreaseVel(acc);
    }

};

/* Overloading function for Body object */
ostream& operator<<(ostream& os, const Body &body) 
{
    os << "Body name : " << body.name << endl;
    os << "Body orbit : " << body.orbit << endl;
    os << "Body mass : " << body.mass << endl;
    os << body.properties;
    return os;
}


// ----- Implement class solar system here ----
// --------------------------------------------
// SolarSystem class variable will be private vector of type class Body called bodies

class SolarSystem {
    // Solar have one variable that is vector of type Body called bodies
    // all planet Body you read should save in this vector

    private:
        vector<Body> bodies;
        unordered_map<string, double> bodies_mass;

        void InitPrimaryBodies(void) 
        {
            for (Body& elm : bodies) {
                string primary_body_name = elm.GetOrbit();

                if (primary_body_name == "NaN") {
                    continue;
                }

                double val = 0;
                auto it = bodies_mass.find(primary_body_name);

                if (it != bodies_mass.end()) {
                    val = it->second;
                } else {
                    val = 0;
                }
                elm.properties.SetPrimaryBodyMass(val);

            }
        }

    public:
        // SolarSystem main function should take the location of .dat file
        // and read the complete file
        SolarSystem(string datapath) {

            ifstream solarsystfile (datapath);

            if (solarsystfile.is_open()) {

                // define variable you want to read some info will be saved and some will be used in calculation
                /* 
                 * Later theses variables will be converted to proper usable values 
                 */

                struct CelestialSpecs spec;

                // don't forget to skip first line since it contain title only
                /* 
                   This call skip the (firstline) header line 
                   that describe fields in the .dat
                */
                getline(solarsystfile, spec.str_name, '\n');

                // read file line by line
                while(!solarsystfile.eof()) {                    
                    solarsystfile >> spec.str_name;
                    solarsystfile >> spec.str_orbit;
                    solarsystfile >> spec.str_mass;
                    solarsystfile >> spec.str_diam;
                    solarsystfile >> spec.str_perihelion;
                    solarsystfile >> spec.str_aphelion;
                    solarsystfile >> spec.str_orbperiod;
                    solarsystfile >> spec.str_rotationalperiod;
                    solarsystfile >> spec.str_axialtilt;
                    solarsystfile >> spec.str_orbinclin;


                    /* 
                        This call will create a body, 
                        generate the initial pos and compute
                        the initial acceleration and velocity.

                        then it will push that bodies inside the vector
                    */
                    Body new_planet = Body(spec.str_name, spec.str_orbit, stod(spec.str_mass), BodyCelestialProperties(spec));
                    bodies.push_back(new_planet);
                    bodies_mass.insert({spec.str_name, stod(spec.str_mass)});
                }
                solarsystfile.close();

                InitPrimaryBodies();

                for (auto& body : bodies) {
                    double ang = GenerateAngle(0, 2*pi);
                    body.properties.SetBodyMotion(ang);
                }

            } else {
                cout << "Can't open the file at : " << datapath << endl;
            }

        }

    // you will need to save Sun mass to use in planet velocity calculation
    // only calculate the orbiting velocity for planet orbiting sun ,e.g., if you read Io moon that orbiting Venus you skip the calculation
    // print out planet: name  Orbit: Sun
    // print out the orbit velocity
    // print out centripetal acceleration
    // All calculation is simplified
    // check out this wiki https://en.wikipedia.org/wiki/Circular_orbit
    

    // For x, y, and z of AstroVec3d pos"position" generate random number between 0 and 10
    // look up how to use rand function



    // don't forget to close the open .dat file after reading the complete file


    // add the value of accelerations in stepForward
    // this function take variable bodies inside Solarsystem and int acc and use it with function stepAcceleration
    // function should be void
    // stepForward()

    void stepAccelForward(int acc) {
       for (auto& elm : bodies) {
            elm.stepAcceleration(acc);
       }
    }

    // BONUS : Step up velocity as well
    void stepVeloForward(int vel) {
       for (auto& elm : bodies) {
            elm.stepVelocity(vel);
       }
    }

    // overload SolarSystem object, so it loops through list of bodies and cout it
    // this function also depend on the overload of Body class
    friend ostream& operator<<(ostream& os, const SolarSystem &system);

};

/* Overloading << function for SolarSystem object */
ostream& operator<<(ostream& os, const SolarSystem &system)
{
    for (auto& elm : system.bodies) {
        os << "______________________________" << endl;
        os << elm;
        os << "______________________________" << endl;
    }

    return os;
}


int main() {

    cout << "########" << endl;
    cout << "Main Problem" << endl;
    cout << "########" << endl;

    // SolarSystem s should read the file to print and do calculation
    // when you creat object Solarsystem s('.dat path') it read the file and print only planets orbiting the sun  as shown below
    //    body name:Jupiter orbit: Sun
    //    orbital velocity is: 13058
    //    centripetal acceleration: 0.000219013
    //    body name:Saturn orbit: Sun
    //    orbital velocity is: 9623.1
    //    centripetal acceleration: 6.45977e-05
    //    ==================
    // Write your fullpath here to location where solarsystem.dat exist.
    
    /* Generate seed for rand based on time entropy */
    srand(time(NULL));

    SolarSystem s ("solarsystem.dat");

    cout << s;
    // After reading the file print calculation done on Solarsystem
    // overload << operator in SolarSystem class to print out variables as follows
    //    Mercury, Sun, 3.3e+23, 8, 10, 4, -2147483648, -2147483648, 0, -1981977299, 1152671425, 0
    //    Venus, Sun, 4.87e+24, 9, 1, 3, -2147483648, -2147483648, 0, 1021382959, -679782887, 0


    cout << "=============" << endl << endl;
    
    
    cout << "Acceleration augmentation " << endl;
    // add acceleration equal 100
    // Use function .stepForward() on object Solarsystem to set the new acceleration
    int acc = 100;
    s.stepAccelForward(acc);
    // print out the new solarsystem
    cout << s;

    cout << "====[ end ]====" << endl;
    cout << "               " << endl;
    
    return 0;

}
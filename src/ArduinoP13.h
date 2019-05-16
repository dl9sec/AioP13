//
// ArduinoP13.h
//
// An implementation of Plan13 in C++ by Mark VandeWettering
//
// Plan13 is an algorithm for satellite orbit prediction first formulated
// by James Miller G3RUH.  I learned about it when I saw it was the basis 
// of the PIC based antenna rotator project designed by G6LVB.
//
// http://www.g6lvb.com/Articles/LVBTracker2/index.htm
//
// I ported the algorithm to Python, and it was my primary means of orbit
// prediction for a couple of years while I operated the "Easy Sats" with 
// a dual band hand held and an Arrow antenna.
//
// I've long wanted to redo the work in C++ so that I could port the code
// to smaller processors including the Atmel AVR chips.  Bruce Robertson,
// VE9QRP started the qrpTracker project to fufill many of the same goals,
// but I thought that the code could be made more compact and more modular,
// and could serve not just the embedded targets but could be of more
// use for more general applications.  And, I like the BSD License a bit
// better too.
//
// So, here it is!
//
// =====================================================================
//
// dl9sec@gmx.net (02..05/2019):
//
// The original Plan13 BBC Basic source code can be found at:
//
// https://www.amsat.org/articles/g3ruh/111.html
//
// Published as "Donationware" in favour of AMSAT-UK, LONDON, E12 5EQ
// and the AO-13 Amateur Satellite Program.
//
// Changes:
// - Refactoring for seamless use with Arduino.
// - Renamed class DateTime to P13DateTime because of potential conflict
//   with RTClib, which uses the same class name.
// - Renamed all classes to P13... for consistency and clarification of
//   affiliation.
// - Some code beautification.
// - Fixed missing semicolon at destructor of P13DateTime.
// - Added comments for constants and TLE data.
// - Updated sidereal and solar data to values valid until ~2030
// - Used all double
// - Used PI instead of M_PI
// - Used degrees() and radians() instead of DEGREES() and RADIANS()
// - Inserted explicit casts
// - Added a method "footprint" to class P13Satellite
// - Added a method "doppler" to calculate down- and uplink frequencies
//
//----------------------------------------------------------------------

#ifndef Arduino_P13_H
#define Arduino_P13_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#define P13_FRX	0
#define P13_FTX	1


static long fnday(int y, int m, int d);
static void fndate(int &y, int &m, int &d, long dt);
static double getdouble(const char *c, int i0, int i1);
static long getlong(const char *c, int i0, int i1);
void latlon2xy(int &x, int &y, double lat, double lon, const int MapMaxX, const int MapMaxY);

//----------------------------------------------------------------------

// here are a bunch of constants that will be used throughout the 
// code, but which will probably not be helpful outside.

static const double RE = 6378.137;				// WGS-84 Earth ellipsoid
static const double FL = 1.0 / 298.257224;		// -"-
static const double RP = RE * (1.0 - FL);		// -

static const double GM = 3.986E5;				// Earth's Gravitational constant km^3/s^2
static const double J2 = 1.08263E-3;			// 2nd Zonal coeff, Earth's Gravity Field

static const double YM = 365.25;				// Mean Year,     days
static const double YT = 365.2421874;			// Tropical year, days
static const double WW = 2.0 * PI / YT; 		// Earth's rotation rate, rads/whole day
static const double WE = 2.0 * PI + WW;			// Earth's rotation rate, radians/day 
static const double W0 = WE / 86400.0;			// Earth's rotation rate, radians/sec

// Sidereal and Solar data. Rarely needs changing. Valid to year ~2030
static const double YG = 2014.0;				// GHAA, Year YG, Jan 0.0
static const double G0 = 99.5828;				// -"-
static const double MAS0 = 356.4105;			// MA Sun and rate, deg, deg/day
static const double MASD = 0.98560028;			// -"-
static const double INS = radians(23.4375);		// Sun's inclination
static const double CNS = cos(INS);				// -"-
static const double SNS = sin(INS);				// -"-
static const double EQC1 = 0.03340;				// Sun's Equation of centre terms
static const double EQC2 = 0.00035;				// -"-

//----------------------------------------------------------------------

// The original BASIC code used three variables (e.g. Ox, Oy, Oz) to
// represent a vector quantity.  I think that makes for slightly more
// obtuse code, so I going to collapse them into a single variable 
// which is an array of three elements.

typedef double Vec3[3];

//----------------------------------------------------------------------

class P13DateTime {

public:
	long   DN;
 	double TN;
	
   	P13DateTime();
	P13DateTime(const P13DateTime &);
	P13DateTime(int year, int month, int day, int h, int m, int s);
	~P13DateTime();
	
	void add(double);
   	void settime(int year, int month, int day, int h, int m, int s);
    void gettime(int& year, int& mon, int& day, int& h, int& m, int& s);
    void ascii(char *);
	void roundup(double);
};


//----------------------------------------------------------------------

class P13Observer {

public:
    const char *name;
    double LA;
    double LO;
    double HT;
	
    Vec3 U, E, N, O, V;
    
    P13Observer(const char *nm, double lat, double lon, double asl);
    ~P13Observer();
};


//----------------------------------------------------------------------

class P13Satellite { 

public:
    const char *name;
	
	Vec3 SAT, VEL;		// Celestial coordinates
    Vec3 S, V; 			// Geocentric coordinates
 
	P13Satellite(const char *name, const char *l1, const char *l2);
	~P13Satellite();
	
    void   tle(const char *name, const char *l1, const char *l2);
    void   predict(const P13DateTime &dt);
 	void   latlon(double &lat, double &lon);
	void   elaz(const P13Observer &obs, double &el, double &az);
	void   footprint(int points[][2], int numberofpoints, const int MapMaxX, const int MapMaxY, double &satlat, double &satlon);
	double doppler(double freqMHz, bool dir);

private:
  	long   N;		// Satellite calaog number
	long   YE;		// Epoch Year    			year
	double TE;		// Epoch time    			days
	double IN;		// Inclination   			deg
	double RA;		// R.A.A.N.      			deg
	double EC;		// Eccentricity  			 -
	double WP;		// Arg perigee   			deg
	double MA;		// Mean anomaly  			deg
	double MM;		// Mean motion   			rev/d
	double M2;		// Decay Rate    			rev/d/d
	double RV;		// Orbit number  			 -
	double ALON;	// Sat attitude				deg
	double ALAT;	// Sat attitude				deg
    long   DE;		// Epoch Fraction of day
	
	// These values are stored, but could be calculated on the fly during calls to predict() 
	// Classic space/time tradeoff

    double N0, A_0, B_0;
    double PC;
    double QD, WD, DC;

    double RS;		
	double RR;		// Range rate for doppler calculation

};


//----------------------------------------------------------------------

class P13Sun {

public:
	Vec3 SUN, H;
	
	P13Sun();
	~P13Sun();
	
    void predict(const P13DateTime &dt);
 	void latlon(double &lat, double &lon);
	void elaz(const P13Observer &obs, double &el, double &az);
};

#endif	// Arduino_P13_H
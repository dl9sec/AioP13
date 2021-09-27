//
// ArduinoP13.cpp
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
// VE9QRP started the qrpTracker project to fulfill many of the same goals,
// but I thought that the code could be made more compact and more modular,
// and could serve not just the embedded targets but could be of more
// use for more general applications.  And, I like the BSD License a bit
// better too.
//
// So, here it is!
//
// =====================================================================
//
// dl9sec@gmx.net (02..09/2021):
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
// - Added a method "footprint" to class P13Sun
// - Added imolementation suggestion for P13Sun:elaz() by Uwe Nagel
//
// Uwe Nagel (09/2021)
//
// - Implementation of P13Sun:elaz()
//
//----------------------------------------------------------------------

#include "ArduinoP13.h"


//----------------------------------------------------------------------
//  _  _     _                  __              _   _             
// | || |___| |_ __  ___ _ _   / _|_  _ _ _  __| |_(_)___ _ _  ___
// | __ / -_) | '_ \/ -_) '_| |  _| || | ' \/ _|  _| / _ \ ' \(_-<
// |_||_\___|_| .__/\___|_|   |_|  \_,_|_||_\__|\__|_\___/_||_/__/
//            |_|                                                 
//
//----------------------------------------------------------------------

// Convert date to day-number

static long fnday(int y, int m, int d) {
    
    if ( m < 3 )
    {
        m += 12;
        y--;
    }
    
    return ((long)((double)y * YM) + (long)((double)(m + 1) * 30.6) + (long)(d - 428));
}


// Convert day-number to date; valid 1900 Mar 01 - 2100 Feb 28

static void fndate(int &y, int &m, int &d, long dt) {
    
    dt += 428L;
    y   = (int)(((double)dt - 122.1) / YM);
    dt -= (long)((double)y * YM);
    m   = (int)((double)dt / 30.61);
    dt -= (long)((double)m * 30.6);
    m--;
    
    if ( m > 12 )
    {
        m -= 12;
        y++;
    }
    
    d = (int)dt;
}


// Cut characters from c with start i0 to i1-1, append string
// delimiter and convert to double

static double getdouble(const char *c, int i0, int i1) {
    
    char buf[20];
    int i;
    
    for (i=0; i0+i<i1; i++) {
        buf[i] = c[i0+i];
    }
    
    buf[i] = '\0';
    
    return (strtod(buf, NULL));
}


// Cut characters from c with start i0 to i1-1, append string
// delimiter and convert to long

static long getlong(const char *c, int i0, int i1) {
    
    char buf[20];
    int i;
    
    for (i = 0; (i0 + i) < i1; i++) {
        buf[i] = c[i0+i];
    }
    
    buf[i] = '\0';
    
    return (atol(buf));
}


// Converts latitude (Breitengrad) -90..90° / longitude (Laengengrad) -180..180°
// to x/y-coordinates of a map with maxamimum dimension MapMaxX * MapMaxY
void latlon2xy(int &x, int &y, double lat, double lon, const int MapMaxX, const int MapMaxY) {
    
    x = (int)((((180.0 + lon) / 360.0) * (double)MapMaxX));
    y = (int)(((( 90.0 - lat) / 180.0) * (double)MapMaxY));
    
}


//----------------------------------------------------------------------
//     _              ___  _ _______       _      _____ _           
//  __| |__ _ ______ | _ \/ |__ /   \ __ _| |_ __|_   _(_)_ __  ___ 
// / _| / _` (_-<_-< |  _/| ||_ \ |) / _` |  _/ -_)| | | | '  \/ -_)
// \__|_\__,_/__/__/ |_|  |_|___/___/\__,_|\__\___||_| |_|_|_|_\___|
//                                                       
//----------------------------------------------------------------------

P13DateTime::P13DateTime() {
    
   DN = 0L;
   TN = 0.0;
}


P13DateTime::P13DateTime(const P13DateTime &dt) {
    
    DN = dt.DN;
    TN = dt.TN;
}


P13DateTime::P13DateTime(int year, int month, int day, int h, int m, int s) {

    settime(year, month, day, h, m, s);
}


P13DateTime::~P13DateTime() {
    
}


void P13DateTime::add(double days) {
    
    TN += days;
    DN += (long)TN;
    TN -= (long)TN;
}

//OK
void P13DateTime::settime(int year, int month, int day, int h, int m, int s) {
    
    DN = fnday(year, month, day);
    TN = ((double)h + (double)m / 60.0 + (double)s / 3600.0) / 24.0;
}


void P13DateTime::gettime(int &year, int &month, int &day, int &h, int &m, int &s) {
    
    double t;

    fndate(year, month, day, DN);
    t  = TN;
    t *= 24.0;
    h  = (int)t;
    t -= (double)h;
    t *= 60.0;
    m  = (int)t;
    t -= (double)m ;
    t *= 60.0;
    s  = (int)(t);
}


void P13DateTime::ascii(char *buf) {
    
    int year, mon, day;
    int h, m, s;
    
    gettime(year, mon, day, h, m, s);
    // 2019-05-11 00:53:13
    sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d", year, mon, day, h, m, s);
}


void P13DateTime::roundup(double t) {
    
    double inc;
    
    inc = t - fmod(TN, t);
    
    TN += inc;
    DN += (long)TN;
    TN -= (long)TN;
}


//----------------------------------------------------------------------
//     _              ___  _ ____ ___  _                            
//  __| |__ _ ______ | _ \/ |__ // _ \| |__ ___ ___ _ ___ _____ _ _ 
// / _| / _` (_-<_-< |  _/| ||_ \ (_) | '_ (_-</ -_) '_\ V / -_) '_|
// \__|_\__,_/__/__/ |_|  |_|___/\___/|_.__/__/\___|_|  \_/\___|_|  
//                                                      
//----------------------------------------------------------------------
// Set the observers parameters from latitude, longitude and altitude
// above sea level (m).

P13Observer::P13Observer(const char *nm, double lat, double lon, double asl) {
    
    double D, CL, SL, CO, SO;
    double Rx;
    double Rz;
    
    this->name = nm;
    LA = radians(lat);
    LO = radians(lon);
    HT = asl / 1000.0;

    CL = cos(LA);
    SL = sin(LA);
    CO = cos(LO);
    SO = sin(LO);

    U[0] = CL * CO;
    U[1] = CL * SO;
    U[2] = SL;

    E[0] = -SO;
    E[1] =  CO;
    E[2] =  0.0;

    N[0] = -SL * CO;
    N[1] = -SL * SO;
    N[2] =  CL;

    D = sqrt(RE * RE * CL * CL + RP * RP * SL * SL);
    Rx = (RE * RE) / D + HT;
    Rz = (RP * RP) / D + HT;

    O[0] = Rx * U[0];
    O[1] = Rx * U[1];
    O[2] = Rz * U[2];

    V[0] = -O[1] * W0;
    V[1] =  O[0] * W0;
    V[2] =  0.0;

}


P13Observer::~P13Observer() {
    
}

//----------------------------------------------------------------------
//     _              ___  _ _______       _       _ _ _ _       
//  __| |__ _ ______ | _ \/ |__ / __| __ _| |_ ___| | (_) |_ ___ 
// / _| / _` (_-<_-< |  _/| ||_ \__ \/ _` |  _/ -_) | | |  _/ -_)
// \__|_\__,_/__/__/ |_|  |_|___/___/\__,_|\__\___|_|_|_|\__\___|
//
//----------------------------------------------------------------------

P13Satellite::P13Satellite(const char *nm, const char *l1, const char *l2) {
    
    tle(nm, l1, l2);
}

P13Satellite::~P13Satellite() {
    
}

// Get satellite data from the TLE

void P13Satellite::tle(const char *nm, const char *l1, const char *l2) {
    
    double CI;
    
    name = nm;

    // Direct quantities from the orbital elements

    N  = getlong(l1,  2,  7);               // Get satellite catalog number from tle:l1:2..6
    YE = getlong(l1, 18, 20);               // Get epoch year from tle:l1:18..19
        
    if ( YE < 58 )
        YE += 2000;
    else
        YE += 1900;

    TE = getdouble(l1, 20, 32);             // Get epoch (day of the year and fractional portion of the day) from tle:l1:20..31
    M2 = 2.0 * PI * getdouble(l1, 33, 43);  // Get first time derivative of the mean motion divided by to from tle:l1:33..42

    IN = radians(getdouble(l2, 8, 16));     // Get inclination (degrees) from tle:l2:8..15
    RA = radians(getdouble(l2, 17, 25));    // Get R.A.A.N (degrees) from tle:l2:17..24
    EC = getdouble(l2, 26, 33) / 1.0E7;     // Get eccentricity from tle:l2:26..32
    WP = radians(getdouble(l2, 34, 42));    // Get argument of perigee (degrees) from tle:l2:34..41
    MA = radians(getdouble(l2, 43, 51));    // Get mean anomaly (degrees) from tle:l2:43..50
    MM = 2.0 * PI * getdouble(l2, 52, 63);  // Get mean motion from tle:l2:52..62
    RV = getlong(l2, 63, 68);               // Get Revolution number at epoch (revolutions) from tle:l2:63..67

    // Derived quantities from the orbital elements 

    // convert TE to DE and TE 
    DE = fnday(YE, 1, 0) + (long)TE;
    
    TE -= (long)TE;

    N0  = MM / 86400.0;
    A_0 = pow(GM / (N0 * N0), 1.0/3.0);
    B_0 = A_0 * sqrt(1.0 - EC * EC);
    
    PC  = RE * A_0 / (B_0 * B_0);
    PC  = 1.5 * J2 * PC * PC * MM;
    
    CI = cos(IN);
    QD = -PC * CI;
    WD =  PC * (5.0 * CI * CI - 1.0) / 2.0;
    DC = -2.0 * M2 / (3.0 * MM);
}


void P13Satellite::predict(const P13DateTime &dt) {
    
    long   DN;
    
    double TN;
    double GHAE, GHAA;
    double T, DT, KD, KDP;
    double M, DR, RN, EA;
    double DNOM, C_EA, S_EA;
    double A, B, D;
    double AP, CW, SW;
    double RAAN;
    double CQ, SQ;
    double CI, SI;
    double CG, SG;
    
    Vec3 CX, CY, CZ;
    
    DN = dt.DN;
    TN = dt.TN;

    GHAE = radians(G0) + ((double)(DE - fnday(YG, 1, 0)) + TE) * WE;    // GHA Aries, epoch

    T   = (double)(DN - DE) + (TN - TE);    // Elapsed T since epoch, days
    DT  = DC * T / 2.0;                     // Linear drag terms
    KD  = 1.0 + 4.0 * DT;                   // -"-
    KDP = 1.0 - 7.0 * DT;                   // -"-
  
    M   = MA + MM * T * (1.0 - 3.0 * DT);   // Mean anomaly at YR,TN
    DR  = (long)(M / (2.0 * PI));           // Strip out whole no of revs
    M  -= DR * 2.0 * PI;                    // M now in range 0..2PI
    RN  = RV + DR;                          // Current Orbit number
    
    // Solve M = EA - EC*SIN(EA) for EA given M, by Newton's Method
    EA  = M;                                // Initial solution

    do
    {
        C_EA = cos(EA);
        S_EA = sin(EA);
        DNOM = 1.0 - EC * C_EA;
        D = (EA - EC * S_EA - M) / DNOM;    // Change to EA for better solution
        EA -= D ;                           // by this amount
    }
    while (fabs(D) > 1.0E-5);

    // Distances
    A = A_0 * KD;           
    B = B_0 * KD;
    RS = A * DNOM;

    // Calc satellite position & velocity in plane of ellipse
    S[0] = A * (C_EA - EC);
    S[1] = B * S_EA;
    
    V[0] = -A * S_EA / DNOM * N0;
    V[1] =  B * C_EA / DNOM * N0;

    AP = WP + WD * T * KDP;
    CW = cos(AP);
    SW = sin(AP);
    RAAN = RA + QD * T * KDP;
    CQ = cos(RAAN);
    SQ = sin(RAAN);

    // CX, CY, and CZ form a 3x3 matrix that converts between orbit
    // coordinates, and celestial coordinates.
    
    // Plane -> celestial coordinate transformation, [C] = [RAAN]*[IN]*[AP]
    CI = cos(IN);
    SI = sin(IN);    
   
    CX[0] =  CW * CQ - SW * CI * SQ;
    CX[1] = -SW * CQ - CW * CI * SQ;
    CX[2] =  SI * SQ;

    CY[0] =  CW * SQ + SW * CI * CQ;
    CY[1] = -SW * SQ + CW * CI * CQ;
    CY[2] = -SI * CQ;

    CZ[0] = SW * SI;
    CZ[1] = CW * SI;
    CZ[2] = CI;

    // Compute SATellite's position vector and VELocity in
    // CELESTIAL coordinates. (Note: Sz=S[2]=0, Vz=V[2]=0)
    SAT[0] = S[0] * CX[0] + S[1] * CX[1];
    SAT[1] = S[0] * CY[0] + S[1] * CY[1];
    SAT[2] = S[0] * CZ[0] + S[1] * CZ[1];

    VEL[0] = V[0] * CX[0] + V[1] * CX[1];
    VEL[1] = V[0] * CY[0] + V[1] * CY[1];
    VEL[2] = V[0] * CZ[0] + V[1] * CZ[1];

    // Also express SAT and VEL in GEOCENTRIC coordinates:
    GHAA = (GHAE + WE * T); // GHA Aries at elapsed time T
    CG   = cos(-GHAA);
    SG   = sin(-GHAA);

    S[0] = SAT[0] * CG - SAT[1] * SG;
    S[1] = SAT[0] * SG + SAT[1] * CG;
    S[2] = SAT[2];

    V[0] = VEL[0] * CG - VEL[1]* SG;
    V[1] = VEL[0] * SG + VEL[1]* CG;
    V[2] = VEL[2];
}


void P13Satellite::latlon(double &lat, double &lon) {
    
    lat = degrees(asin(S[2] / RS));
    lon = degrees(atan2(S[1], S[0]));
}


void P13Satellite::elaz(const P13Observer &obs, double &el, double &az) {
    
    double r, u, e, n;
    
    Vec3 R; // Rangevec

    
    // Rangevec = Satvec - Obsvec
    R[0] = S[0] - obs.O[0];
    R[1] = S[1] - obs.O[1];
    R[2] = S[2] - obs.O[2];
    
    // Range magnitude
    r = sqrt(R[0] * R[0] + R[1] * R[1] + R[2] * R[2]);
    
    // Normalise Range vector
    R[0] /= r;
    R[1] /= r;
    R[2] /= r;
    
    // UP Component of unit range
    u = R[0] * obs.U[0] + R[1] * obs.U[1] + R[2] * obs.U[2];
    // EAST
    e = R[0] * obs.E[0] + R[1] * obs.E[1];
    //NORTH
    n = R[0] * obs.N[0] + R[1] * obs.N[1] + R[2] * obs.N[2];
    
    // Azimuth
    az = degrees(atan2(e, n));
    
    if (az < 0.0)
        az += 360.0;
    
    // Elevation
    el = degrees(asin(u));
    
    // Calculate range rate needed for doppler calculation
    // Resolve Sat-Obs velocity vector along unit range vector. (VOz=obs.V[2]=0)
    RR  = (V[0] - obs.V[0]) * R[0] + (V[1] - obs.V[1]) * R[1] + V[2] * R[2];    // Range rate, km/s
    
}

// Generates the footprint for a satellite at satlat/satlon and calculates rectangular
// x/y coordinates scaled to a map with size MapMaxX/MapMaxY. The coordinates are stored
// in a two dimensional array. points[n][0] stores x and points[n][1] stores y.
// The coordinates can be concatenated with lines to create a footprint outline.

void P13Satellite::footprint(int points[][2], int numberofpoints, const int MapMaxX, const int MapMaxY, double &satlat, double &satlon) {

    int i;
    
    double srad;
    double cla, sla, clo, slo;
    double sra, cra;
    
    double a, x, y, z, Xfp, Yfp, Zfp;
    
        
    srad = acos(RE / RS);   // Radius of footprint circle
    sra  = sin(srad);       // Sin/Cos these to save time
    cra  = cos(srad);

    cla  = cos(radians(satlat));
    sla  = sin(radians(satlat));
    clo  = cos(radians(satlon));
    slo  = sin(radians(satlon));
    
    for ( i = 0; i < numberofpoints ; i++)  // "numberofpoints" points to the circle
    {
        a = 2.0 * PI * (double)i / (double)numberofpoints;  // Angle around the circle
        Xfp = cra;                                          // Circle of points centred on Lat=0, Lon=0
        Yfp = sra * sin(a);                                 // assuming Earth's radius = 1
        Zfp = sra * cos(a);
        
        x   = Xfp * cla - Zfp * sla;                        // Rotate point "up" by latitude "satlat"
        y   = Yfp;                                          // -"-
        z   = Xfp * sla + Zfp * cla;                        // -"-
        
        Xfp = x * clo - y * slo;                            // Rotate point "around" through longitude "satlon"
        Yfp = x * slo + y * clo;                            // -"-
        Zfp = z;                                            // -"-
        
        // Convert point to Lat/Lon and convert/scale to a pixel map
        latlon2xy(points[i][0], points[i][1], degrees(asin(Zfp)), degrees(atan2(Yfp,Xfp)), MapMaxX, MapMaxY);
    }

}


// Returns the RX (dir = 0 or P13_FRX) or TX (dir = 1 or P13_FTX) frequency with doppler shift.
double P13Satellite::doppler(double freqMHz, bool dir) {
    
    double dopplershift;    // Dopplershift in MHz
    
    dopplershift = -freqMHz * RR / 299792.0;    //  Speed of light is 299792.0 km/s
    
    if (dir)    // TX
    {
        freqMHz = freqMHz - dopplershift;
    }
    else        // RX
    {
        freqMHz = freqMHz + dopplershift;
    }

    return (freqMHz);

}


//----------------------------------------------------------------------
//     _              ___  _ _______           
//  __| |__ _ ______ | _ \/ |__ / __|_  _ _ _  
// / _| / _` (_-<_-< |  _/| ||_ \__ \ || | ' \ 
// \__|_\__,_/__/__/ |_|  |_|___/___/\_,_|_||_|
//                                             
//----------------------------------------------------------------------

P13Sun::P13Sun() {
    
}


P13Sun::~P13Sun() {
    
}


void P13Sun::predict(const P13DateTime &dt) {
    
    long   DN;
    
    double TN;
    double T, GHAE, MRSE, MASE, TAS;
    double C, S;
    
    DN = dt.DN;
    TN = dt.TN;

    T    = (double)(DN - fnday(YG, 1, 0)) + TN;
    GHAE = radians(G0) + T * WE;
        
    MRSE = radians(G0) + T * WW + PI;
    MASE = radians(MAS0 + T * MASD);
    TAS  = MRSE + EQC1 * sin(MASE) + EQC2 * sin(2.0 * MASE);

    // Sin/Cos Sun's true anomaly
    C = cos(TAS);
    S = sin(TAS);

    // Sun unit vector - CELESTIAL coords
    SUN[0] = C;
    SUN[1] = S * CNS;
    SUN[2] = S * SNS;
    
    // Obtain SUN unit vector in GEOCENTRIC coordinates
    C = cos(-GHAE); 
    S = sin(-GHAE); 
        
    H[0] = SUN[0] * C - SUN[1] * S;
    H[1] = SUN[0] * S + SUN[1] * C;
    H[2] = SUN[2];
}


void P13Sun::latlon(double &lat, double &lon) {
    
    lat = degrees(asin(H[2]));
    lon = degrees(atan2(H[1], H[0]));
}

// Testing in progress...
void P13Sun::elaz(const P13Observer &obs, double &el, double &az) {
    
    // Copyright (c) 2021 Uwe Nagel
    
    double r, u, e, n;
    
	Vec3 R; // Rangevec
    
    // Rangevec = Satvec - Obsvec
    R[0] = H[0] * AU - obs.O[0] ;
    R[1] = H[1] * AU - obs.O[1] ;
    R[2] = H[2] * AU - obs.O[2] ;
    
    // Range magnitude
    r = sqrt(R[0] * R[0] + R[1] * R[1] + R[2] * R[2]);
    
    // Normalise Range vector
    R[0] /= r;
    R[1] /= r;
    R[2] /= r;
    
    // UP Component of unit range
    u = R[0] * obs.U[0] + R[1] * obs.U[1] + R[2] * obs.U[2];
    // EAST
    e = R[0] * obs.E[0] + R[1] * obs.E[1];
    //NORTH
    n = R[0] * obs.N[0] + R[1] * obs.N[1] + R[2] * obs.N[2];
    
    // Azimuth
    az = degrees(atan2(e, n));

    if (az < 0.0)
        az += 360.0;
    
    // Elevation
    el = degrees(asin(u));

}


// Generates the sunlight footprint at satlat/satlon and calculates rectangular
// x/y coordinates scaled to a map with size MapMaxX/MapMaxY. The coordinates are stored
// in a two dimensional array. points[n][0] stores x and points[n][1] stores y.
// The coordinates can be concatenated with lines to create a footprint outline.
//
// This is a simplified aproach with no real calculation of the distance to the sun at a
// specific time. It is assumed that the nearest and farest distance of the sun makes almost no
// difference in footprint radius, it is always almost 0.5*PI. Therefore one astronomical
// unit is used for the distance. The same algorithm is used as for the satellite footprint
// except, that RS is replaced by AU.

void P13Sun::footprint(int points[][2], int numberofpoints, const int MapMaxX, const int MapMaxY, double &sunlat, double &sunlon) {

    int i;
    
    double srad;
    double cla, sla, clo, slo;
    double sra, cra;
    
    double a, x, y, z, Xfp, Yfp, Zfp;
    
        
    srad = acos(RE / AU);   // Radius of sunlight footprint circle
    sra  = sin(srad);       // Sin/Cos these to save time
    cra  = cos(srad);

    cla  = cos(radians(sunlat));
    sla  = sin(radians(sunlat));
    clo  = cos(radians(sunlon));
    slo  = sin(radians(sunlon));
    
    for ( i = 0; i < numberofpoints ; i++)  // "numberofpoints" points to the circle
    {
        a = 2.0 * PI * (double)i / (double)numberofpoints;  // Angle around the circle
        Xfp = cra;                                          // Circle of points centred on Lat=0, Lon=0
        Yfp = sra * sin(a);                                 // assuming Earth's radius = 1
        Zfp = sra * cos(a);
        
        x   = Xfp * cla - Zfp * sla;                        // Rotate point "up" by latitude "sunlat"
        y   = Yfp;                                          // -"-
        z   = Xfp * sla + Zfp * cla;                        // -"-
        
        Xfp = x * clo - y * slo;                            // Rotate point "around" through longitude "sunlon"
        Yfp = x * slo + y * clo;                            // -"-
        Zfp = z;                                            // -"-
        
        // Convert point to Lat/Lon and convert/scale to a pixel map
        latlon2xy(points[i][0], points[i][1], degrees(asin(Zfp)), degrees(atan2(Yfp,Xfp)), MapMaxX, MapMaxY);
    }

}

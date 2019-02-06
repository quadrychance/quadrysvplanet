/**
  @file body.c

  @brief Relationships between parameters associated with individual bodies.

  @author Rory Barnes ([RoryBarnes](https://github.com/RoryBarnes/))

  @date May 7 2014

  This file contains subroutines that describe physical properties of
  any body. This include conversions between the option parameter (a property
  that may be used at input) and the system parameter (the property in the BODY
  struct that is always up-to-date). If unsure between here and orbit.c, put
  here. Also includes mathemtatical relationships.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "vplanet.h"

/*
 * Mathematical Relationships
 */

/**
  Calculate the sign of a number

  @param dValue The number whose sign is to be calculated
  @param iSign The sign of the number

  @return the sign (-1,0, or 1).
*/
int fiSign(double dValue) {
  int iSign;

  if (fabs(dValue) > EPS) // EPS set in vplanet.h
    iSign = (int)(dValue/fabs(dValue));
  else
    iSign = 0;

  return iSign;
}

/**
  Calcaulte the derivative of a period.

  @param dRotRate The rotational frequency
  @param dDrotrateDt The time derivative of the rotational frequency

  @return The time derivative of the rotational frequency
*/
double fdDPerDt(double dRotRate,double dDrotrateDt) {
    return -2*PI*dDrotrateDt/(dRotRate*dRotRate);
}

/**
  Caclulate the characteristic timescale for a variable to change that is
  controlled by only 1 module.

  @param dVar The value of the variable
  @param dDeriv The value of the variable's time derivative

  @return The timescale of the variable's change: |x/(dx/dt)|. If the
  derivative is 0, return 0.
*/
double fdTimescale(double dVar,double dDeriv) {
  if (dDeriv != 0)
    return fabs(dVar/dDeriv);
  else
    return 0;
}

/**
Caclulate the characteristic timescale for a variable to change that is
controlled by multiple processes.

@param dVar The value of the variable
@param dDeriv Array of the values of the variable's time derivatives
@param iNum The number of derivatives
@param dTime Dummy variable to keep track of the sum of the derivative
@param iPert Index of the perturbing process

@return The timescale of the variable's change: |x/Sum(dx/dt)|
*/
double fdTimescaleMulti(double dVar,double *dDeriv,int iNum) {
  double dTime;
  int iPert;

  dTime=0;
  for (iPert=0;iPert<iNum;iPert++) {
    if (dDeriv[iPert] != 0)
      dTime += dDeriv[iPert]; // Note that here dTime is actullay the rate
    dTime = fabs(dVar/dTime);
  }
  return dTime;
}

/**
 Convert an angular frequency to a period

 @param dFreq The frequency
 @return The period
*/
double fdFreqToPer(double dFreq) {
  return 2*PI/dFreq;
}

/**
  Convert a period to an angular frequency

  @param dPeriod The period
  @return The frequency
*/
double fdPerToFreq(double dPeriod) {
  return 2*PI/dPeriod;
}

/*
 * Physical Relationships
 */

/**
  Calculate a body's gravitational potential energy

  @param dMass The body's mass
  @param dRadius The body's radius

  @return The body's potential energy
*/
double fdBodyPotEnergy(double dMass, double dRadius) {
  /* ALPHA_STRUCT  is structural constant for spherical mass distribution
     potential energy (E_pot = -ALPHA*BIGG*M^2/R = 0.6), see vplanet.h. */
  return -ALPHA_STRUCT*BIGG*dMass*dMass/dRadius;
}

/**
  Calculate a body's rotational angular momentum

  @param dRadGyra Body's radius of gyration
  @param dMass Body's mass
  @param dRad Body's radius
  @param dOmega Body's rotational frequency

  @return Body's rotational angular momentum
*/
double fdRotAngMom(double dRadGyra,double dMass,double dRad,double dOmega) {
  return dRadGyra*dRadGyra*dMass*dRad*dRad*dOmega;
}

/**
  Calculate a body's rotational kinetic energy

  @param dMass Body's mass
  @param dRadius Body's radius
  @param dRadyGyra Body's radius of gyration
  @param dOmega Body's rotational frequency

  @return Body's rotational jinetic energy
*/
double fdRotKinEnergy(double dMass,double dRadius,double dRadGyra,double dOmega) {
  return 0.5*dRadGyra*dRadGyra*dMass*dRadius*dRadius*dOmega*dOmega;
}

/**
  Convert a rotational frequency to a rotational velocity

  @param dRadius Body's radius
  @param dFreq Body's rotational frequency

  @return Body's rotational velocity
*/
double fdRadiusFreqToRotVel(double dRadius,double dFreq) {
  return dRadius*dFreq;
}

/**
  Convert rotational velocity to rotational frequency

  @param dRotVel Body's rotational velocity
  @param dRadius Body's radius

  @return Body's rotational frequency
*/
double fdRadiusRotVelToFreq(double dRotVel,double dRadius){
  return dRotVel/dRadius;
}

/**
  Calculate radius from density and mass

  @param dDensity Body's bulk density
  @param dMass Body's mass

  @return Body's Radius
*/
double fdDensityMassToRadius(double dDensity,double dMass) {
  return pow( (3*dDensity/(4*PI*dMass)), (1./3) );
}

/**
  Calculate Mass from Radius and density

  @param dRadius Body's radius
  @param dDensity Body's bulk density

  @return Body's mass
*/
double fdMassFromRadiusDensity(double dRadius,double dDensity){
  return 4*PI*pow(dRadius,3)/(3*dDensity);
}

/**
  Calculate rotational velocity from radius and rotation frequency

  @param dRadius Body's radius
  @param dRotRate Body's rotational frequency

  @return Body's rotational velocity
*/
double fdRotVel(double dRadius,double dRotRate) {
  return dRadius*dRotRate;
}

/**
 Calculate density for a uniform sphere

 @param dMass Body's mass
 @param Body's radius

 @return Body's bulk density
*/
double fdSphereDensity(double dMass,double dRadius) {
  return 4*PI*dMass/pow(dRadius,3)/3;
}

/*
 * Published Mass - Radius Relationships
 */

/**
  Stellar mass-radius relationship from New Light on Dark Stars, Table 4.1.
  See Barnes et al. (2013) Astrobiology 13:225-250.

  @param dRadius Stellar radius
  @param x Log base-10 of the stellar radius in solar units
  @param y Log base-10 of the stellar mass in solar units

  @return Stellar Mass
*/
double fdRadToMass_ReidHawley(double dRadius) {
  double x,y;

    x = log10(dRadius/RSUN);
    y = 0.1277 + 2.185*x + 3.135*x*x + 1.9031*x*x*x;

    return pow(10,y)*MSUN;
}

/**
  Stellar mass-radius relationship from New Light on Dark Stars, Table 4.1.
  See Barnes et al. (2013) Astrobiology 13:225-250.

  @param dMass Stellar mass
  @param x Log base-10 of the stellar radius in solar units
  @param y Log base-10 of the stellar mass in solar units

  @return Stellar Radius
*/
double fdMassToRad_ReidHawley(double dMass) {
  double x,y;

  x = log10(dMass/MSUN);
  y = 0.1424 + 1.568*x - 0.2342*x*x - 0.5581*x*x*x;

  return pow(10,y)*RSUN;
}

/**
  Stellar mass-radius relationship from Gorda, S. Yu. & Svechnikov, M. A. 1999,
  Astronomy Reports, 43, 521-525.

  @param dMass Stellar Mass
  @return Stellar radius
*/
double fdMassToRad_GordaSvech99(double dMass) {
  dMass = log10(dMass/MSUN);

  if (dMass > 0.14)
    return pow(10,(0.096 + 0.652*log10(dMass)))*RSUN;
  else
    return pow(10,(0.1 + 1.03*log10(dMass)))*RSUN;
}

/**
  Stellar mass-radius relationship from Gorda, S. Yu. & Svechnikov, M. A. 1999,
  Astronomy Reports, 43, 521-525. Reverse fit from Barnes et al. (2013)
  Astrobiology 13:225-250.

  @param dRadius Stellar Radius
  @param x Log base-10 of the stellar radius in solar units
  @param y Log base-10 of the stellar mass in solar units
  @return Stellar mass
*/
double fdRadToMass_GordaSvech99(double dRadius) {
    double x,y;

    x = log10(dRadius/RSUN);
    y = -0.09709 + 0.9709*x - 2.502e-5*x*x - 1.34e-5*x*x*x;

    return pow(10,y);
}


/**
  Stellar mass-radius relationship from Bayless, A.J. & Orosz, J.A. 2006,
  ApJ, 651, 1155-1165.

  @param dMass Stellar Mass
  @param dRadius Stellar radius is solar units

  @return Stellar radius
*/
double fdMassToRad_BaylessOrosz06(double dMass) {
    double dRadius;

    dMass = dMass/MSUN;
    dRadius = 0.0324 + 0.9343*dMass + 0.0374*dMass*dMass;

    return dRadius*RSUN;
}

/**
  Stellar mass-radius relationship from Bayless, A.J. & Orosz, J.A. 2006,
  ApJ, 651, 1155-1165.

  @param dRadius Stellar Radius
  @param dMasss Stellar masss is solar units

  @return Stellar masss
*/
double fdRadToMass_BaylessOrosz06(double dRadius) {
    double dMass;

    dRadius = dRadius/RSUN;
    dMass = -0.03477 + 1.07146*dRadius - 8.171*dRadius*dRadius -0.0412*dRadius*dRadius*dRadius;

    return dMass*MSUN;
}


/**
  Terrestrial planet mass-radius relationship from Sotin et al 2007, Icarus,
  191, 337-351.

  @param dMass Planetary mass
  @return Planetary radius
*/
double fdMassToRad_Sotin07(double dMass) {
    return pow(dMass/MEARTH,0.272)*REARTH;
}

/**
  Terrestrial planet mass-radius relationship from Sotin et al 2007, Icarus,
  191, 337-351.

  @param dRadius Planetary radius
  @return Planetary mass
*/
double fdRadToMass_Sotin07(double dRadius) {
    return pow(dRadius/REARTH,3.6765)*MEARTH;
}

/**
 Assign Radius based on mass and published relationship

 @param dMass Body's mass
 @param iRelation ID of mass-radius relationship to use

 @return Body's radius as provided by appropriate relationship
*/
double fdMassToRad(double dMass,int iRelation) {

  if (iRelation == REIDHAWLEY)
    return fdMassToRad_ReidHawley(dMass);
  else if (iRelation == GORDASVECH99)
    return fdMassToRad_GordaSvech99(dMass);
  else if (iRelation == BAYLESSOROSZ06)
    return fdMassToRad_BaylessOrosz06(dMass);
  else if (iRelation == SOTIN07)
    return fdMassToRad_Sotin07(dMass);

  /* Need to add more! */

  /* Whoops! */
  fprintf(stderr,"ERROR: Unknown mass-radius relationship.\n");
  fprintf(stderr,"Mass: %.3e, Relationship: %d\n",dMass,iRelation);
  exit(EXIT_UNITS);
}

// Assign mass from radius and published relationship
double fdRadToMass(double dMass,int iRelation) {

  if (iRelation == REIDHAWLEY)
    return fdRadToMass_ReidHawley(dMass);
  else if (iRelation == GORDASVECH99)
    return fdRadToMass_GordaSvech99(dMass);
  else if (iRelation == BAYLESSOROSZ06)
    return fdRadToMass_BaylessOrosz06(dMass);
  else if (iRelation == SOTIN07)
    return fdRadToMass_Sotin07(dMass);

  /* Need to add more! XXX */

  /* Whoops! */
  return 1./0;
}

/**
 Copy the body struct from src to dest

 @param dest Struct to receive the src
 @param src Struct that contains original information
 */
void BodyCopy(BODY *dest,BODY *src,EVOLVE *evolve) {
  int iBody,iModule;

  /* This subroutine only includes parameters needed for more than 1 module,
     Module-specific parameters belong in the fnBodyCopy subroutines. */

  for (iBody=0;iBody<evolve->iNumBodies;iBody++) {
    dest[iBody].iBodyType = src[iBody].iBodyType;
    dest[iBody].dMass = src[iBody].dMass;
    dest[iBody].dRadius = src[iBody].dRadius;
    dest[iBody].dRadGyra = src[iBody].dRadGyra;
    dest[iBody].dXobl = src[iBody].dXobl;
    dest[iBody].dYobl = src[iBody].dYobl;
    dest[iBody].dZobl = src[iBody].dZobl;
    dest[iBody].dRotRate = src[iBody].dRotRate;
    dest[iBody].dAge = src[iBody].dAge;
    dest[iBody].dEcc = src[iBody].dEcc; // XXX iBody=0 could be in galhabit?
    dest[iBody].dPrecA = src[iBody].dPrecA;
    dest[iBody].dObliquity = src[iBody].dObliquity;
    dest[iBody].dLostAngMom = src[iBody].dLostAngMom;
    dest[iBody].dLostEng = src[iBody].dLostEng;
    dest[iBody].bBinary = src[iBody].bBinary;
    dest[iBody].bDistOrb = src[iBody].bDistOrb;
    dest[iBody].bDistRot = src[iBody].bDistRot;
    dest[iBody].bEqtide = src[iBody].bEqtide;
    dest[iBody].bFlare = src[iBody].bFlare;
    dest[iBody].bGalHabit = src[iBody].bGalHabit;
    dest[iBody].bPoise = src[iBody].bPoise;
    dest[iBody].bStellar = src[iBody].bStellar;
    dest[iBody].bThermint = src[iBody].bThermint;
    dest[iBody].bRadheat = src[iBody].bRadheat;
    dest[iBody].bSpiNBody = src[iBody].bSpiNBody;

    //dest[iBody].dLXUV = src[iBody].dLXUV;

    // Im(k_2) properties
    dest[iBody].dK2Man = src[iBody].dK2Man;
    dest[iBody].dTidalQMan = src[iBody].dTidalQMan;
    dest[iBody].dImK2Man = src[iBody].dImK2Man;
    // These copies are needed to avoid floating point exceptions in
    //   (eqtide + !thermint) bodies
    dest[iBody].dShmodUMan=src[iBody].dShmodUMan;
    dest[iBody].dStiffness=src[iBody].dStiffness;

    if (iBody > 0) {
      dest[iBody].dHecc = src[iBody].dHecc;
      dest[iBody].dKecc = src[iBody].dKecc;
      dest[iBody].dSemi = src[iBody].dSemi;
      dest[iBody].dRadius = src[iBody].dRadius;
      dest[iBody].dMeanMotion = src[iBody].dMeanMotion;
    }
    /* Copy module-specific properties */
    for (iModule=0;iModule<evolve->iNumModules[iBody];iModule++)
      evolve->fnBodyCopy[iBody][iModule](dest,src,evolve->iEqtideModel,evolve->iNumBodies,iBody);
  }
}

/**
 Calculate rotational variables from obliquity and precession angle

 @param body Body struct
 @param iBody Index of the body struct for the body's whose rotational spins
  is to be calculated.
  */
void CalcXYZobl(BODY *body, int iBody) {
  body[iBody].dXobl = sin(body[iBody].dObliquity)*cos(body[iBody].dPrecA);
  body[iBody].dYobl = sin(body[iBody].dObliquity)*sin(body[iBody].dPrecA);
  body[iBody].dZobl = cos(body[iBody].dObliquity);
}

/**
 Calculate equilibrium shape of planet using scaling laws and solar system
   values. If the value is less then Venus', return Venus'.

   @param body BODY struct
   @param iBody Index of the body struct for the body's whose equilibrium shape
    is to be calculated.
   @param J2Earth Earth's current oblateness
   @param J2Venus Venus' current oblateness
   @param CEarth Earth's current moment of inertia?
   @param nuEarth ???
   @param EdEarth ???
   @param dTmp ???
   @param dDynEllip Dynamical ellipticity

   @return Dynamical elliptiticy
   */
double CalcDynEllipEq(BODY *body, int iBody) {
  double J2Earth = 1.08262668e-3, J2Venus = 4.56e-6, CEarth = 8.034e37;
  double nuEarth, EdEarth, EdVenus, dTmp, dDynEllip;

  EdEarth = J2Earth*MEARTH*pow(REARTH,2)/CEarth;
  EdVenus = J2Venus/0.336;
  nuEarth = 2*PI/(DAYSEC);

  dTmp = EdEarth*MEARTH/(pow(nuEarth,2)*pow(REARTH,3));

  dDynEllip = dTmp*pow(body[iBody].dRotRate,2)*pow(body[iBody].dRadius,3)/body[iBody].dMass;

  if (dDynEllip < EdVenus)
    dDynEllip = EdVenus;

  return dDynEllip;
}

/**
  Lehmer+ (2017)'s model for the radius of a planet where it's losing its atmopshere
   due to XUV radiation.

   @param dMassEnv Envelope's mass
   @param dGravAccel Body's gravitational acceleration
   @param dRadSurf Surface Radius of rocky core
   @param dScaleHeight Atmospheric scale height
   @param dPresSurf pressure at surface due to envelope
   @param dRadXUV radius from center of planet where optical depth of XUV is unity
   */
double fdLehmerRadius(double dRadSurf, double dPresXUV, double dScaleHeight, double dPresSurf) {
	double dRadXUV;

	dRadXUV = dRadSurf * dRadSurf / (dScaleHeight * log(dPresXUV/dPresSurf) + dRadSurf);
	if (dRadXUV <= dRadSurf) {
		dRadXUV = dRadSurf;
	}
  return dRadXUV;
}

/**
  Lehmer+ (2017)'s model for the pressure of a planet where it's losing its atmopshere
   due to XUV radiation.

   @param dMassEnv Envelope's mass
   @param dGravAccel Body's gravitational acceleration
   @param dRadSurf Surface Radius of rocky core
   @param dPresXUV Pressure at base of thermosphere
   @param dScaleHeight Atmospheric scale height
   @param dPresSurf pressure at surface due to envelope
   */
double fdLehmerPres(double dMassEnv, double dGravAccel, double dRadSurf) {
	double dPresSurf;

	dPresSurf = dGravAccel * dMassEnv / (4 * PI * dRadSurf * dRadSurf); // [kg/ms2]
  return dPresSurf;
}

double fdImK2Total(BODY *body,int iBody) {

  if (body[iBody].bMantle || body[iBody].bOcean || body[iBody].bEnv) {
    return body[iBody].dImK2Man + body[iBody].dImK2Ocean + body[iBody].dImK2Env;
  } else {
    return body[iBody].dK2/body[iBody].dTidalQ;
  }
}

/**
  Function compute upper mantle k2 Love number

  @param body Body struct
  @param iBody Index of body

  @return Upper mantle k2 Love number
*/
double fdK2Man(BODY *body,int iBody) {
  return 1.5/(1+9.5*body[iBody].dShmodUMan/(STIFFNESS));
}

double fdTidalQMan(BODY *body,int iBody) {
  return body[iBody].dDynamViscos*body[iBody].dMeanMotion/body[iBody].dShmodUMan;
}


double fdImK2ManThermint(BODY *body,int iBody) {
  double dDenom2;

  dDenom2 = pow(1.5*body[iBody].dTidalQMan/body[iBody].dK2Man,2.);
  return (57./4)*body[iBody].dDynamViscos*body[iBody].dMeanMotion/((body[iBody].dStiffness)*(1.0+dDenom2));
}


/**
  Function compute upper mantle imaginary component of k2 Love number

  @param body Body struct
  @param iBody Index of body

  @return Imaginary component of k2 Love number
*/
double fdImK2Man(BODY *body,int iBody) {

  if (body[iBody].bThermint) {
    body[iBody].dK2 = fdK2Man(body,iBody);
    return fdImK2ManThermint(body,iBody);
  } else {
    return body[iBody].dImK2Man;
  }
}

/** Calculate total k_2 and Im(k_2) for a body. This value depends on which tidal
  model is called, and it the properties depend on the internal properties.  XXX Cut?
void AssignTidalProperties(BODY *body,EVOLVE *evolve,int iBody) {
  if (body[iBody].bThermint) {
    // The planet's tidal response depends on mantle properties
    body[iBody].dK2=fdK2Man(body,iBody);
    body[iBody].dImK2=fdImK2Man(body,iBody);
  } else {
     We're in a CPL/CTL type mode, and let's start building the total K2 and
      ImK2 with the mantle. This should be sorted out in Verify.
    body[iBody].dK2 = body[iBody].dK2Man;
    body[iBody].dImK2 = body[iBody].dImK2Man;
  }
  // Include tidal dissapation due to oceans
  if (body[iBody].bOceanTides) {
    // weighted by the love number of each component
    body[iBody].dK2 += body[iBody].dK2Ocean;
    body[iBody].dImK2 += body[iBody].dImK2Ocean;
  }
  // Include tidal dissapation due to envelope
  if (body[iBody].bEnvTides) {
    // weighted by the love number of each component
    body[iBody].dK2 += body[iBody].dK2Env;
    body[iBody].dImK2 += body[iBody].dImK2Env;
  }
  // Sanity checks: enforce upper bound
  if (body[iBody].dK2 > 1.5) {
    body[iBody].dK2 = 1.5;
    fprintf(stderr,"WARNING: body[%d].dK2 > 1.5 at time %.5e years.\n",iBody,evolve->dTime/YEARSEC);
  }
}
*/

/**
  Function compute secular mantle heat flow: heat sinks - sources

  @param body Body struct
  @param iBody Index of body

  @return Heat flow of erupted mantle melt
*/
double fdHflowSecMan(BODY *body,EVOLVE *evolve,int iBody) {
  double dHflowSecMan = 0;

  if (body[iBody].bThermint) {
    dHflowSecMan += fdPowerThermint(body,iBody);
  }
  if (body[iBody].bEqtide) {
    dHflowSecMan -= fdTidePower(body,iBody,evolve->iEqtideModel); // formerly dTidalPowerMan
  }
  // Should add RadHeat here
  return dHflowSecMan;
}

/**
  For use with `fdProximaCenStellar()` to interpolate stellar properties
  (temperature, radius, luminosity) from a grid.

  @param dVal Value of (temperature, radius, luminosity)
  @param daArr Array of values from Yonsei-Yale tracks
  @param iDim Length of array
  @param iIndex Index of dArr correpoinding to the value

  @return iIndex
*/
int fiGetLowerBoundProximaCen(double dVal, const double *daArr, int iDim){
	int iIndex;
	for (iIndex=0;iIndex<iDim-2;iIndex++){
	  if (dVal < daArr[iIndex+1]) break;
  }
	return iIndex;
}

/**
  For use with `fdProximaCenStellar()` to interpolate stellar properties
  (temperature, radius, luminosity) from a grid. This function
  linearly interpolates over data, given indices of lower bounds on grid xi, yi
  and normalized distances to the interpolation point dx, dy.

  XXX What are these arguments?
*/
double fdProximaCenBiLinear(int iALEN, double const data_lo[iALEN], double const data_hi[iALEN], int xi, int yi, double dx, double dy) {
	double C0, C1, C;
	if (dx == 0) {
	  C0 = data_lo[xi];
	  C1 = data_hi[xi];
	} else {
	  C0 = data_lo[xi]*(1-dx) + data_lo[xi+1]*dx;
	  C1 = data_hi[xi]*(1-dx) + data_hi[xi+1]*dx;
	}
	if (dy == 0)
	  C = C0;
	else
	  C = C0*(1-dy) + C1*dy;
	return C;
}

/**
For use with `fdProximaCenStellar()` to interpolate stellar properties
(temperature, radius, luminosity) from a grid.

  XXX What are these arguments?
*/
double fdProximaCenInterpolate(int iALEN, int iMLEN, double const xarr[iALEN], double const yarr[iMLEN], double const data_lo[iALEN], double const data_hi[iALEN], double A, double M, int *iError){
  double dx,dy;
  int xi,yi;
  int dxi,dyi;
  double result = 0;

  // Let's enforce a minimum age of 0.001 GYR and a maximum age of 10.0 GYR
  // NOTE: This results in a constant luminosity at times beyond this range.
  if (A < 0.001) A = 0.001;
  if (A > 10.00) A = 10.00;

  // Bounds on mass
  if (M < 0.1) {
		*iError = STELLAR_ERR_OUTOFBOUNDS_LO;
		return 0;
	} else if (M > 0.15) {
		*iError = STELLAR_ERR_OUTOFBOUNDS_HI;
		return 0;
	}

  // Get the lower bound
  xi = fiGetLowerBoundProximaCen(log10(A),xarr,iALEN);
  yi = fiGetLowerBoundProximaCen(M,yarr,iMLEN);

  // Paranoia check (REMOVE)
  if (yi < 0) {
    *iError = STELLAR_ERR_OUTOFBOUNDS_LO;
    return 0;
  } else if (yi > 1) {
    *iError = STELLAR_ERR_OUTOFBOUNDS_HI;
    return 0;
  }

  // Normalized distance to grid points
  dx = (log10(A)-xarr[xi]) / (xarr[xi + 1] - xarr[xi]);
  dy = (M-yarr[yi]) / (yarr[yi + 1] - yarr[yi]);

  // Calculate
  result = fdProximaCenBiLinear(iALEN,data_lo,data_hi,xi,yi,dx,dy);
  if (isnan(result)) {
    *iError = PROXIMACEN_ERROR;
    return 0;
  }
  *iError = 0;
  return result;
}

/**
Computes the temperature, luminosity, or radius of Proxima Centauri
by interpolating from a grid. The isochrones are from the Y^2 website

http://www.astro.yale.edu/demarque/fs255_grid.tar.gz

with [Fe/H] = +0.3 track and mixing length parameter
alpha_MLT = 1.0. These were linearly interpolated between the 0.1 MSUN and
0.15 MSUN tracks to create luminosity, temperature, and radius
interpolants, which are functions of a single variable (the age).
The grid was then rectified and copied as constant arrays to `body.h`.

Note that in order to match the present-day luminosity (see below),
I had to fudge a bit. I ended up scaling the entire luminosity
track down by 15% to get it to match. I then re-computed the
temperature to be consistent with the radius. Now all three
quantities (L, R, and T) match within less than 1 sigma.

DATA FROM Boyajian+12; SECOND ROW FROM Demory+09 (direct measurements)

# Fe/H      Radius            Luminosity            Teff          Mass            Lx/Lbol
# 0.19      0.1410 ± 0.0070   0.00155 ± 0.00002     3054 ± 79     0.118           2.83E−04
                              0.00165 ± 0.00015     3098 ± 56     0.123 ± 0.006

  XXX What are these arguments?

*/
double fdProximaCenStellar(int iParam, double A, double M, int *iError) {
	double res;
	double dLum, dRad;

  if (iParam == PROXIMACEN_T) {
      // Get fudged luminosity
      dLum = fdProximaCenInterpolate(PROXIMACEN_ALEN, PROXIMACEN_MLEN, PROXIMACEN_AARR, PROXIMACEN_MARR, PROXIMACEN_LOGL_LO, PROXIMACEN_LOGL_HI, A / (1.e9 * YEARSEC), M / MSUN, iError);
      dLum = LSUN * pow(10., dLum * PROXIMACEN_FUDGE);
      // Get radius
      dRad = fdProximaCenInterpolate(PROXIMACEN_ALEN, PROXIMACEN_MLEN, PROXIMACEN_AARR, PROXIMACEN_MARR, PROXIMACEN_LOGR_LO, PROXIMACEN_LOGR_HI, A / (1.e9 * YEARSEC), M / MSUN, iError);
      dRad = RSUN * pow(10., dRad);
      // Compute self-consistent temperature
      res = pow(dLum / (4 * PI * dRad * dRad * SIGMA), 0.25);
      return res;
  } else if (iParam == PROXIMACEN_L) {
      res = fdProximaCenInterpolate(PROXIMACEN_ALEN, PROXIMACEN_MLEN, PROXIMACEN_AARR, PROXIMACEN_MARR, PROXIMACEN_LOGL_LO, PROXIMACEN_LOGL_HI, A / (1.e9 * YEARSEC), M / MSUN, iError);
      return LSUN * pow(10., res * PROXIMACEN_FUDGE);
  } else if (iParam == PROXIMACEN_R) {
      res = fdProximaCenInterpolate(PROXIMACEN_ALEN, PROXIMACEN_MLEN, PROXIMACEN_AARR, PROXIMACEN_MARR, PROXIMACEN_LOGR_LO, PROXIMACEN_LOGR_HI, A / (1.e9 * YEARSEC), M / MSUN, iError);
      return RSUN * pow(10., res);
  } else {
      *iError = PROXIMACEN_ERROR;
      return 0;
  }
}



/**
For use with `fdProximaCenBRadius()` to interpolate the radius of
Proxima Cen b from a grid, assuming it has a gaseous composition

@param dVal Value of (temperature, radius, luminosity)
@param daArr Array of values from Yonsei-Yale tracks
@param iDim Length of array
@param iIndex Index of dArr correpoinding to the value

@return iIndex
*/
int fiGetLowerBoundProximaCenB(double dVal, const double *daArr, int iDim){
	int iIndex;
	for (iIndex=0;iIndex<iDim-2;iIndex++){
	  if (dVal < daArr[iIndex+1]) break;
  }
	return iIndex;
}

/**
  For use with `fdProximaCenBRadius()` to interpolate the radius of
  Proxima Cen b from a grid, assuming it has a gaseous composition

  XXX What are the arguments?
*/
double fdProximaCenBLinear(int xi, int yi, double dx, double dy) {
	// Linearly interpolate over data, given indices of lower bounds on grid xi, yi
	// and normalized distances to the interpolation point dx, dy.
	double C0, C1, C;
	if (dx == 0) {
	  C0 = daProxCenBRadius[xi][yi];
	  C1 = daProxCenBRadius[xi][yi+1];
	} else {
	  C0 = daProxCenBRadius[xi][yi]*(1-dx) + daProxCenBRadius[xi+1][yi]*dx;
	  C1 = daProxCenBRadius[xi][yi+1]*(1-dx) + daProxCenBRadius[xi+1][yi+1]*dx;
	}
	if (dy == 0)
	  C = C0;
	else
	  C = C0*(1-dy) + C1*dy;

	return C;
}

/**
For use with `fdProximaCenBRadius()` to interpolate the radius of
Proxima Cen b from a grid, assuming it has a gaseous composition

Here I'm assuming a mass of 1.27 MEARTH and a solid body radius of 1.074 REARTH.
I'm using the Lopez+12 grids from Luger et al. (2015)
and smoothing over sharp (presumably) numerical features.

  XXX What are the arguments?
*/
double fdProximaCenBRadius(double C, double A, double M){
	double dx, dy;
	int xi, yi;

	// Let's enforce a minimum age of 0.001 GYR and a maximum age of 10.0 GYR
	A /= YEARSEC;
	if (A < 1e7) A = 1e7;
	if (A > 1e10) A = 1e10;

	// If the planet is rocky, use the Sotin+07 relation
	if (C < 0.00001)
	  return fdMassToRad_Sotin07(M);

	// Let's enforce the bounds for the composition as well
	if (C > 0.01) C = 0.01;

	// Get the lower bounds
	xi = fiGetLowerBoundProximaCenB(C, daProxCenBComp, PROXCENBCOMPLEN);
	yi = fiGetLowerBoundProximaCenB(A, daProxCenBAge, PROXCENBTIMELEN);

	// Normalized distance to grid points
	dx = (C - daProxCenBComp[xi]) / (daProxCenBComp[xi + 1] - daProxCenBComp[xi]);
	dy = (A - daProxCenBAge[yi]) / (daProxCenBAge[yi + 1] - daProxCenBAge[yi]);

  // Calculate
  return fdProximaCenBLinear(xi,yi,dx,dy) * REARTH;
}

/**
  Planet radius evolution from the Lopez et al. (2012) evolution grids.

  XXX What are the arguments?

*/
double fdLopezRadius(double dMass, double dComp, double dFlux, double dAge, int iMetal){
	int m, c, f, t, z;
	double dm, dc, df, dt;
	double R000,R001,R010,R011,R100,R101,R110,R111;
	double R00,R10,R01,R11;
	double R0,R1;
	double dMassEarth = dMass/MEARTH;
	double dAgeYears = dAge/YEARSEC;

	/* We're not going to bother interpolating between metallicities. */
	z = iMetal;

	// Add a small tolerance
	if ((dMassEarth/daLopezMass[0] < 1)) {
        /* Out of bounds, assuming it's OK to use min val */
        dMassEarth = daLopezMass[0];
        m = 0;
    } else if ((dMassEarth/daLopezMass[MASSLEN-1] > 1)) {
        /* Out of bounds, assuming it's OK to use max val */
        dMassEarth = daLopezMass[MASSLEN-1];
        m = MASSLEN-1;
	} else {
		/* Get index just below desired mass */
		for (m = 0; m < MASSLEN-1; m++)
			if (dMassEarth < daLopezMass[m+1]) break;
	}
	if (dComp < daLopezComp[0]){
		/* Out of bounds, assuming it's OK to use min val */
		dComp = daLopezComp[0];
		c = 0;
	} else if (dComp >= daLopezComp[COMPLEN-1]){
        /* Out of bounds, assuming it's OK to use max val */
		dComp = daLopezComp[COMPLEN-1];
		c = COMPLEN - 1;
	} else {
		/* Get index just below desired composition */
		for (c = 0; c < COMPLEN-1; c++)
			if (dComp < daLopezComp[c+1]) break;
	}
	if (dFlux < daLopezFlux[0]){
		/* Out of bounds, assuming it's OK to use min val */
		dFlux = daLopezFlux[0];
		f = 0;
	} else if (dFlux >= daLopezFlux[FLUXLEN-1]){
		/* Out of bounds, assuming it's OK to use max val */
		dFlux = daLopezFlux[FLUXLEN-1];
		f = FLUXLEN - 1;
	} else {
		/* Get index just below desired composition */
		for (f = 0; f < FLUXLEN-1; f++)
			if (dFlux < daLopezFlux[f+1]) break;
	}
	if (dAgeYears < daLopezAge[0]){
		/* Out of bounds, assuming it's OK to use min val */
		dAgeYears = daLopezAge[0];
		t = 0;
	} else if (dAgeYears >= daLopezAge[TIMELEN-1]){
		/* Out of bounds, assuming it's OK to use max val */
		dAgeYears = daLopezAge[TIMELEN-1];
		t = TIMELEN - 1;
	} else {
		/* Get index just below desired time */
		for (t = 0; t < TIMELEN-1; t++)
			if (dAgeYears < daLopezAge[t+1]) break;
	}
	/* We now have the coordinates below our desired point.
	 * Let's use them to do a simple tetralinear interpolation.
	 * Adapted from the method described in
	 * http://en.wikipedia.org/wiki/Trilinear_interpolation */
	dm = (dMassEarth - daLopezMass[m])/(daLopezMass[m+1] - daLopezMass[m]);
	dc = (dComp - daLopezComp[c])/(daLopezComp[c+1] - daLopezComp[c]);
	df = (dFlux - daLopezFlux[f])/(daLopezFlux[f+1] - daLopezFlux[f]);
	dt = (dAgeYears - daLopezAge[t])/(daLopezAge[t+1] - daLopezAge[t]);
	R000 = daLopezRadius[m][c][f][z][t]*(1-dm) + daLopezRadius[m+1][c][f][z][t]*dm;
	R001 = daLopezRadius[m][c][f][z][t+1]*(1-dm) + daLopezRadius[m+1][c][f][z][t+1]*dm;
	R010 = daLopezRadius[m][c][f+1][z][t]*(1-dm) + daLopezRadius[m+1][c][f+1][z][t]*dm;
	R011 = daLopezRadius[m][c][f+1][z][t+1]*(1-dm) + daLopezRadius[m+1][c][f+1][z][t+1]*dm;
	R100 = daLopezRadius[m][c+1][f][z][t]*(1-dm) + daLopezRadius[m+1][c+1][f][z][t]*dm;
	R101 = daLopezRadius[m][c+1][f][z][t+1]*(1-dm) + daLopezRadius[m+1][c+1][f][z][t+1]*dm;
	R110 = daLopezRadius[m][c+1][f+1][z][t]*(1-dm) + daLopezRadius[m+1][c+1][f+1][z][t]*dm;
	R111 = daLopezRadius[m][c+1][f+1][z][t+1]*(1-dm) + daLopezRadius[m+1][c+1][f+1][z][t+1]*dm;
	R00 = R000*(1-dc) + R100*dc;
	R10 = R010*(1-dc) + R110*dc;
	R01 = R001*(1-dc) + R101*dc;
	R11 = R011*(1-dc) + R111*dc;
	R0 = R00*(1-df) + R10*df;
	R1 = R01*(1-df) + R11*df;
	return (R0*(1-dt) + R1*dt)*REARTH;
}

/**
  Dot product of two vectors

  @param x First array
  @param y Second array
  @param res dot product
  @return dot product of arrays x and y
*/
double fdDotProduct(const int *x, const double *y){
	double res = 0.0;
	int i;
	for (i = 0; i < 16; i++){
		res += x[i] * y[i];
	}
	return res;
}

/**
  Matrix-vector multiplication

  @param mat Matrix
  @param vec Vector
  @param result Resultant vector

*/
void fvMatrixVectorMult(const int mat[16][16], const double *vec, double *result){
	// in matrix form: result = mat * vec;
	int i;
	for (i = 0; i < 16; i++){
		result[i] = fdDotProduct(mat[i], vec);
	}
}

/**
  Helper function for interpolating Baraffe grid

  XXX What are the arguments?

*/
int fiGetLowerBound(double val, const double *arr, int dim){
	int i;
	if (val < arr[0]){
		return STELLAR_ERR_OUTOFBOUNDS_LO;
	} else if (val > arr[dim-1]) {
	  return STELLAR_ERR_OUTOFBOUNDS_HI;
	} else {
			for (i=0;i<dim-2;i++){
				if (val < arr[i+1]) break;
			}
	}
	// Check to see if i-1, i, i+1 and i+2 are all valid indices
	if (i == 0)
		return STELLAR_ERR_OUTOFBOUNDS_LO;
	else if (i>=dim-2)
		return STELLAR_ERR_OUTOFBOUNDS_HI;
	else
		return i;
}

/**
  Helper function for interpolating Baraffe grid

  XXX What are the arguments?
*/
double fdBaraffeBiLinear(int iMLEN, int iALEN, double const data[iMLEN][iALEN], int xi, int yi, double dx, double dy) {
	// Linearly interpolate over data, given indices of lower bounds on grid xi, yi
	// and normalized distances to the interpolation point dx, dy.
	double C0, C1, C;
	if (dx == 0) {
	  C0 = data[xi][yi];
	  C1 = data[xi][yi+1];
	} else {
	  C0 = data[xi][yi]*(1-dx) + data[xi+1][yi]*dx;
	  C1 = data[xi][yi+1]*(1-dx) + data[xi+1][yi+1]*dx;
	}
	if (dy == 0)
	  C = C0;
	else
	  C = C0*(1-dy) + C1*dy;
	return C;
}

/**
  Helper function for interpolating Baraffe grid

  XXX What are the arguments?
*/
double fdBaraffeBiCubic(int iMLEN, int iALEN, double const data[iMLEN][iALEN], int xi, int yi, double dx, double dy) {
	double dvCoeff[16];
	int j,k;
	int ijkn = 0;
  double dypow = 1;
  double result = 0;

	// Linear algebra time!
	// Adapted from http://en.wikipedia.org/wiki/Bicubic_interpolation
	double dvDeriv[16] = {
										// values of the function at each corner
										data[xi][yi],
										data[xi+1][yi],
										data[xi][yi+1],
										data[xi+1][yi+1],

										// values of df/dx at each corner.
										0.5*(data[xi+1][yi]-data[xi-1][yi]),
										0.5*(data[xi+2][yi]-data[xi][yi]),
										0.5*(data[xi+1][yi+1]-data[xi-1][yi+1]),
										0.5*(data[xi+2][yi+1]-data[xi][yi+1]),

										// values of df/dy at each corner.
										0.5*(data[xi][yi+1]-data[xi][yi-1]),
										0.5*(data[xi+1][yi+1]-data[xi+1][yi-1]),
										0.5*(data[xi][yi+2]-data[xi][yi]),
										0.5*(data[xi+1][yi+2]-data[xi+1][yi]),

										// values of d2f/dxdy at each corner.
										0.25*(data[xi+1][yi+1]-data[xi-1][yi+1]-data[xi+1][yi-1]+data[xi-1][yi-1]),
										0.25*(data[xi+2][yi+1]-data[xi][yi+1]-data[xi+2][yi-1]+data[xi][yi-1]),
										0.25*(data[xi+1][yi+2]-data[xi-1][yi+2]-data[xi+1][yi]+data[xi-1][yi]),
										0.25*(data[xi+2][yi+2]-data[xi][yi+2]-data[xi+2][yi]+data[xi][yi])
										};

	fvMatrixVectorMult(STELLAR_BICUBIC_MATRIX,dvDeriv,dvCoeff);
	dypow = 1;
	for(j = 0; j < 4; ++j) {
		result += dypow*(dvCoeff[ijkn] + dx*(dvCoeff[ijkn+1] + dx*(dvCoeff[ijkn+2] + dx*dvCoeff[ijkn+3])));
		ijkn += 4;
		dypow *= dy;
  }
  return result;
}

/**
  Helper function for interpolating Baraffe grid

  XXX What are the arguments?

*/
double fdBaraffeInterpolate(int iMLEN, int iALEN, double const xarr[iMLEN], double const yarr[iALEN], double const data[iMLEN][iALEN], double M, double A, int iOrder, int *iError){
	double dx, dy;
	int xi,yi;
	int dxi, dyi;
  double result = 0;

	// Let's enforce a minimum age of 0.001 GYR
	// NOTE: This results in a constant luminosity at times earlier than this, which
	// is not realistic. Shouldn't be an issue for most planet evolution calculations,
	// since planets typically form after this time, but this issue needs to be
	// revisited eventually.
	if (A < 0.001) A = 0.001;

	// Get bounds on grid
	*iError = 0;
	xi = fiGetLowerBound(M,xarr,iMLEN);
	yi = fiGetLowerBound(A,yarr,iALEN);

	if (xi < 0) {
		*iError = xi;
		return 0;
	} else if (yi < 0) {
		*iError = yi;
		return 0;
	}

	// Normalized distance to grid points
	dx = (M-xarr[xi])/(xarr[xi+1]-xarr[xi]);
	dy = (A-yarr[yi])/(yarr[yi+1]-yarr[yi]);

	if (iOrder == 1) {
		result = fdBaraffeBiLinear(iMLEN,iALEN,data,xi,yi,dx,dy);
		if (isnan(result)) {
			*iError = STELLAR_ERR_ISNAN;
			return 0;
		}
		return result;
	} else if (iOrder == 3) {
		result = fdBaraffeBiCubic(iMLEN,iALEN,data,xi,yi,dx,dy);
		if (isnan(result)) {
			// Maybe we can still linearly interpolate. Let's check:
			if (dx == 0){
        for (dyi = 0; dyi<2; dyi++){
          if (isnan(data[xi][yi+dyi])){
            // Hopeless; you're bounded by
            // a NaN on at least one side
            *iError = STELLAR_ERR_ISNAN;
            return 0;
          }
        }
			} else if (dy == 0){
				for (dxi = 0; dxi<2; dxi++){
					if (isnan(data[xi+dxi][yi])){
						// Hopeless; you're bounded by
						// a NaN on at least one side
						*iError = STELLAR_ERR_ISNAN;
						return 0;
					}
				}
			} else {
        for (dxi = 0; dxi<2; dxi++){
          for (dyi = 0; dyi<2; dyi++){
            if (isnan(data[xi+dxi][yi+dyi])){
              // Hopeless; you're bounded by
              // a NaN on at least one side
              *iError = STELLAR_ERR_ISNAN;
              return 0;
            }
          }
        }
			}
			// We're good! A linear interpolation will save the day.
			*iError = STELLAR_ERR_LINEAR;
			return fdBaraffeBiLinear(iMLEN,iALEN,data,xi,yi,dx,dy);
  	}
  	return result;
	} else {
		*iError = STELLAR_ERR_BADORDER;
		return 0;
	}
}

/**
  Returns the stellar T, L, or R by interpolating over the Baraffe grid
  using either a bilinear (iOrder = 1) or a bicubic (iOrder = 3) interpolation.

  XXX What are the arguments?
*/
double fdBaraffe(int iParam, double A, double M, int iOrder, int *iError) {
	double res;

  if (iParam == STELLAR_T) {
      res = fdBaraffeInterpolate(STELLAR_BAR_MLEN, STELLAR_BAR_ALEN, STELLAR_BAR_MARR, STELLAR_BAR_AARR, DATA_LOGT, M / MSUN, A / (1.e9 * YEARSEC), iOrder, iError);
      return pow(10., res);
  } else if (iParam == STELLAR_L) {
      res = fdBaraffeInterpolate(STELLAR_BAR_MLEN, STELLAR_BAR_ALEN, STELLAR_BAR_MARR, STELLAR_BAR_AARR, DATA_LOGL, M / MSUN, A / (1.e9 * YEARSEC), iOrder, iError);
      return LSUN * pow(10., res);
  } else if (iParam == STELLAR_R) {
      res = fdBaraffeInterpolate(STELLAR_BAR_MLEN, STELLAR_BAR_ALEN, STELLAR_BAR_MARR, STELLAR_BAR_AARR, DATA_RADIUS, M / MSUN, A / (1.e9 * YEARSEC), iOrder, iError);
      return RSUN * res;
  } else if (iParam == STELLAR_RG) {
      res = fdBaraffeInterpolate(STELLAR_BAR_MLEN, STELLAR_BAR_ALEN, STELLAR_BAR_MARR, STELLAR_BAR_AARR, DATA_RG, M / MSUN, A / (1.e9 * YEARSEC), iOrder, iError);
      return res;
  } else {
      *iError = STELLAR_ERR_FILE;
      return 0;
  }
}

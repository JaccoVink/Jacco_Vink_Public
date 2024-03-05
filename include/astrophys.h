#ifndef __ASTROPHYS_H__
#define __ASTROPHYS_H__

/******************************************************************************/
/*
*  FILE        :  astrophys.h
*  AUTHOR      :  Jacco Vink 
*  DESCRIPTION :  Prototyping of some usefull astrophyscal tools 
*                 (see astroTools.c) and definitions of some usefull constants.
*
*
*  COPYRIGHT   : Jacco Vink, SRON, 1995
*/
/******************************************************************************/
/*  MODIFICATIONS :                                                           
*
*/
/******************************************************************************/


/* DEFINITIONS */
#ifndef INFINITY
//#echo define INFINITY
  #define INFINITY            1.0E30
#endif

#define MAGNITUDE_SUN       4.72
#define L_SUN_WATT          3.90E+26
#define L_SUN_ERG_S         3.90E+33
#define R_SUN_M             6.9599E8
#define R_SUN_CM            6.9599E10
#define REF_MAGNITUDE       4.72
#define MASS_SUN_KG         1.989E+30
#define MASS_SUN_G          1.989E+33
		
#define HUBBLE_0            50

#define C_KM_S              2.99792458E+05
#define C_M_S               2.99792458E+08
#define C_CM_S              2.99792458E+10
#define EPSILON0            8.8542E-12      /* el. permitivity in vacuum F/m */
#define K_BOLTZMANN         1.3806503E-23    /* Joule/Kelvin           */
#define K_BOLTZMANN_ERG     1.3806503E-16    /* erg/Kelvin           */
#define H_PLANCK            6.62606876E-34  /* planck 's constant in Joule s */
#define H_PLANCK_ERG        6.62606876E-27  /* planck 's constant in ergs s */
#define SIGMA_SB_SI         5.67051E-8      /* Stefan Boltzmann constant surf*/
#define SIGMA_SB_CGS        5.67051E-5      
#define BB_CONST_CGS        7.56e-15        /* Stefan Boltzmann constant dens*/

#define G_GRAV_CGS          6.67259E-8
#define G_GRAV_SI           6.67259E-11
#define M_PROTON_KG         1.67262158E-27    /* proton mass in kg      */
#define M_PROTON_G          1.67262158E-24    /* idem in gram           */
#define M_PROTON_EV        938.271998E6      /* idem in eV            */
#define M_PROTON_MEV        938.271998        /* idem in MeV            */
#define M_NEUTRON_KG        1.67492716E-27    /* proton mass in kg      */
#define M_NEUTRON_G         1.67492716E-24    /* idem in gram           */
#define M_NEUTRON_MEV       939.565330        /* idem in MeV            */
#define M_ALPHA_G           6.64465723E-24    /* alpha particle mass in g */
#define M_ALPHA_MEV         3727.379378
#define M_ELECTRON_KG       9.10938188E-31    /* electron mass in kg    */
#define M_ELECTRON_G        9.10938188E-28    /* idem in g              */
#define M_ELECTRON_KEV      510.9989          /* idem in keV            */
#define M_ELECTRON_EV       510998.9          /* idem in eV            */
#define E_ELECTRON          1.602176462E-19   /* electron charge in C   */
#define E_ELECTRON_ESU      4.80320427E-10     /* electron charge in C   */
#define R_ELECTRON_CM       2.8179402894E-13  /* classical electron radius */
#define AMU_KG              1.66053886E-27    /* atomic mass unit in kg */
#define AMU_G               1.66053886E-24    /* idem in g              */
#define AMU_EV              931.49432E6

#define M_PI_ZERO_MEV       134.9745        /* PI 0 mass in MeV       */
#define M_PI_ZERO_EV        134.9745E6      /* PI 0 mass in eV        */
#define M_PI_MINUS_MEV      139.5669        /* PI + mass in MeV       */
#define M_PI_MINUS_EV       139.5669E6      /* PI + mass in eV        */
#define M_PI_PLUS_MEV       139.5669        /* PI - mass in MeV       */
#define M_PI_PLUS_EV        139.5669E6      /* PI - mass in eV        */
#define M_PI_MESON_0_MEV    134.9745        /* pi0 rest mass */
#define M_PI_MESON_0_EV     134.9745E6      /* pi0 rest mass */

#define M_MUON_MEV          105.658389      /* Muon mass in MeV       */
#define M_MUON_EV           105.658389E6    /* Muon mass in eV        */

#define EL_RADIUS_CM        2.817940285E-13 /* clasical electron radius */
#define BOHR_RADIUS_CM      5.291772083E-09 /* Bohr radius/Hydrogen atom */
#define ALPHA_FINE          7.297352533E-3      /* fine structure constant */
#define H_IONIZATION_EV     13.6            /* Hydrogen ionization energy */

#define SIGMA_THOMSON_CM2   6.65245854E-25  /* Thomson cross section */

#define FOE                 1.0E44          /* 1 foe = 10^51 erg=10^44 Joule */
#define FOE_ERG             1.0E51          /* 1 foe = 10^51 erg      */

#define EV_KELVIN           1.16049E+04
#define KEV_KELVIN          1.16049E+07
#define ERG_EV              1.602176462E-12
#define ERG_KEV             1.602176462E-09
#define JOULE_EV            1.602176462E-19
#define HZ_KEV              2.417989491E17
#define HZ_EV               2.417989491E14
//#define JY_KEV              2.417965E17 // Jansky's/
//#define JY_EV               2.417965E17 // Jansky's/
#define JANSKY_SI           1.0e-26  // Jansky in Watt/m^2/Hz
#define JANSKY_CGS          1.0e-23  // Jansky in erg/s/cm^2/Hz
#define PARSEC_CM           3.085678E+18
#define PARSEC_M            3.085678E+16
#define PARSEC_KM           3.085678E+13
#define SEC_YEAR            3.1558E+07
#define SEC_DAY             86400
#define KEV_ANGSTROM        12.39854
#define MILLIBARN_SQRCM     1.0E-27 
#define BARN_SQRCM          1.0E-24
#define DEG2RAD             17.45329252E-03
#define RAD2DEG             57.29577951
#define ARCSEC2RAD          4.848136811E-06
#define RAD2ARCSEC          206264.8062

/* Fluorescent yields */
#define FE_FL_YIELD 0.34

/* SOLAR ABUNDANCES OF SOME ELEMENTS */
#define  H_AB	1.0000		/* per definition */
#define HE_AB	9.7724E-02	
#define  C_AB	3.6308E-04
#define  N_AB	1.1220E-04
#define  O_AB	8.5114E-04
#define NE_AB	1.2303E-04
#define NA_AB	2.1380E-06
#define MG_AB   3.8019E-05
#define AL_AB	2.9512E-06
#define SI_AB	3.5481E-05
#define  S_AB	1.6218E-05
#define AR_AB	3.6308E-06
#define CA_AB	2.2909E-06
#define FE_AB	4.6774E-05
#define NI_AB	1.7783E-06

/* useful mathematical numbers:   */
/* #define PI	3.14159265358979323846 */
#define LN2	0.6931472
#define LN10	2.3025851
#define LN10_25  0.92103404



/* Proto typing: */

/* General astronomy  */
double magnitude( double lumin );
double luminosity( double magn );
double modulus(double z, double b);
double planckfunc_cgs( double freq, double temp);
double planckint1_cgs( double freq1, double freq2,  double temp);
double planckint2_cgs( double freq1, double freq2,  double temp);
double planckphot( double freq1, double freq2,  double temp);

/*  Cluster LF */
double schechterLF( double magn, double mstar, double alpha, double nstar);

/* Plasma physics */
double coulombPotential(double temp, double dens, double Z);
double spitzer(double temp, double dens, double m1, double m2, double z1, double z2);
double shockTemperature(double vel, double atomMass, double gamma);
double shockVelocity(double temp, double  atomMass, double gamma);
double maxwell(double en, double kT);


double compton( double freq, double red_freq0 );
double modified_bb(double freq, double temp, double norm );
double int_compton_modbb( double freq, double freq01, double freql, 
			  double gamma, double temp, double norm );
double int_over_gamma(double freq, double freq01, double freq0l, double gamma1,
		      double gamma2, double index, double temp, double norm );

#endif





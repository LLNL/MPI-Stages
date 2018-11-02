/* 

   Exampi private "datatypes.h"  exampi V 0.0

   Authors: Shane Matthew Farmer, Nawrin Sultana, Anthony Skjellum

   This header provides infrastructure to help implement built-in MPI datatypes as of now. 
   Background infrastructure that will help with derived datatypes (user-defined) later.

  
*/
#define _EXAMPI_DATATYPES_H
#ifndef _EXAMPI_DATATYPES_H
#define _EXAMPI_DATATYPES_H

#include <unordered_map>

namespace exampi
{
// coding: must conform to rule of 5 when complete
class Datatype
{
 public:
 Datatype(const MPI_Datatype &_mpi_dt, int _extent, bool _associative, bool _weakassociative, bool _commutative) : mpi_dt(_mpi_dt), extent(_extent), associative(_associative), weakassociative(_weakassociative), commutative(_commutative) {}
  virtual ~Datatype() {}

  // accessors:
  const MPI_Datatype &get_mpi_datatype() const {return mpi_dt;}
  const int get_extent() const {return extent;}
  const bool get_associative() const {return associative;}
  const bool get_weakassociative() const {return weakassociative; }
  const bool get_commutative() const {return commutative;}

 protected:
  MPI_Datatype mpi_dt;
  int extent;
  bool associative;
  bool weakassociative;
  bool commutative;

 private:
};

// coding: must conform to rule of 5 when complete
class DatatypeProperties
{
 public:
  virtual ~DatatypeProperties() {delete singleton; singleton = 0; isInitialized = false;}

  // come back for Rule of 5 conformance on copy/move constructors please...

  static void add_builtin_datatype (Datatype dt) { if(isInitialized == false) 
                                                   { 
						     singleton = new DatatypeProperties;
						     singleton -> builtins = new std::unordered_map<MPI_Datatype,Datatype>; 
						     singleton -> isInitialized = true;
					           }
                                                   singleton -> builtins ->insert({dt.get_mpi_datatype(), dt});
                                                  //clearer: singleton -> builtins.insert(std::make_pair(dt.get_mpi_datatype(), dt));
                                                 }
      
  // accessors:
  // right now, we only have builtin datatypes;
  static const Datatype *get_properties(const MPI_Datatype &dt) {try {return &builtins->at(dt);}catch(std::out_of_range) { return 0;} }

 protected:

  static bool isInitialized;
  static DatatypeProperties *singleton;
  static std::unordered_map<MPI_Datatype, Datatype> *builtins;
  

 private:
  DatatypeProperties() {}

};
bool DatatypeProperties::isInitialized = false;
std::unordered_map<MPI_Datatype,Datatype>  *DatatypeProperties::builtins = 0;

// used by MPI_Init()!
void InitializeDatatypes()
{
  // compare with mpi.h definitions for completeness of checking of initialization.

  DatatypeProperties::add_builtin_datatype(Datatype(MPI_BYTE,           sizeof(unsigned char),  true,  true, true));
  DatatypeProperties::add_builtin_datatype(Datatype(MPI_CHAR,           sizeof(char),           true,  true, true));
#if 0
  DatatypeProperties::add_builtin_datatype(Datatype(MPI_WCHAR,          sizeof(wchar_t),        true,  true, true));
#endif
  DatatypeProperties::add_builtin_datatype(Datatype(MPI_UNSIGNED_CHAR,  sizeof(unsigned char),  true,  true, true));
  DatatypeProperties::add_builtin_datatype(Datatype(MPI_SHORT,          sizeof(short),          true,  true, true));
  DatatypeProperties::add_builtin_datatype(Datatype(MPI_UNSIGNED_SHORT, sizeof(unsigned short), true,  true, true));
  DatatypeProperties::add_builtin_datatype(Datatype(MPI_INT,            sizeof(int),            true,  true, true));
  DatatypeProperties::add_builtin_datatype(Datatype(MPI_UNSIGNED_INT,   sizeof(unsigned int),   true,  true, true));
  DatatypeProperties::add_builtin_datatype(Datatype(MPI_LONG,           sizeof(long),           true,  true, true));
  DatatypeProperties::add_builtin_datatype(Datatype(MPI_UNSIGNED_LONG,  sizeof(unsigned long),  true,  true, true));
  DatatypeProperties::add_builtin_datatype(Datatype(MPI_FLOAT,          sizeof(float),          false, true, true));
  DatatypeProperties::add_builtin_datatype(Datatype(MPI_DOUBLE,         sizeof(double),         false, true, true));
#if 0
  DatatypeProperties::add_builtin_datatype(Datatype(MPI_LONG_DOUBLE,    sizeof(long double),    false, true, true));
#endif

  // add more here as we grow the support set */


}

// used by MPI_Finalize()!
void DeInitializeDatatypes()
{
}


}
#endif



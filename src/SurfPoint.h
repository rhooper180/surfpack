// Project: SURFPACK++
//
// File:       SurfPoint.h
// Author:     Eric Cyr 
// Modified:   Mark Richards
// 
// Description
// + SurfPoint class - a container class for a point and its response values
// + Left shift (<<) operator for SurfPoint. Easy printing.
// ____________________________________________________________________________

#ifndef __SURF_POINT_H__
#define __SURF_POINT_H__

// INVARIANTS: for SurfPoint
// ---------------------------

// not yet specified

class SurfPoint {
  
// ____________________________________________________________________________
// Creation, Destruction, Initialization 
// ____________________________________________________________________________
public:

  /// Initialize without any response values
  SurfPoint(const std::vector<double>& x);

  /// Initialize point with one response value
  SurfPoint(const std::vector<double>& x, double f0);
  
  /// Initialize with zero or more response values
  SurfPoint(const std::vector<double>& x, const std::vector<double>& f);
  
  /// Read point from istream in either text or binary format
  SurfPoint(unsigned xsize, unsigned fsize, std::istream& is, bool binary = false);

  /// Copy constructor performs a deep copy
  SurfPoint(const SurfPoint& sp);

  /// STL data members x and f automatically cleaned up
  ~SurfPoint();

protected:
  /// Default constructor explicitly disallowed.  A SurfPoint must at least
  /// specify a point in some domain space.
  SurfPoint();

// ____________________________________________________________________________
// Overloaded operators 
// ____________________________________________________________________________
public:

  /// Assign sp to this unless they are already identical
  SurfPoint& operator=(const SurfPoint& sp);

  /// Tests for deep equality
  bool operator==(const SurfPoint& sp) const;
  
  /// Tests for deep inequality
  bool operator!=(const SurfPoint& sp) const;

// ____________________________________________________________________________
// Queries 
// ____________________________________________________________________________

  /// Return dimensionality of data point
  unsigned xSize() const;

  /// Return number of response variables
  unsigned fSize() const;

  /// Return point in the domain as an STL vector
  const std::vector<double>& X() const;

  /// Return response value at responseIndex
  double F(unsigned responseIndex = 0) const;

// ____________________________________________________________________________
// Commands 
// ____________________________________________________________________________

  /// Append a new response variable
  unsigned addResponse(double val = 0); 

  /// Set an existing response variable to a new value
  void F(unsigned responseIndex, double responseValue);
// ____________________________________________________________________________
// I/O
// ____________________________________________________________________________

  /// Write point to stream in text or binary format
  std::ostream& write(std::ostream& os, bool binary=false) const;

// ____________________________________________________________________________
// Data members 
// ____________________________________________________________________________

protected:
  /// The point in the domain.  The size of the vector
  /// is the dimensionality of the space. 
  std::vector<double> x;          

  /// Zero or more response values at x (i.e., f1(x), f2(x) ... )
  std::vector<double> f;      

// ____________________________________________________________________________
// Testing 
// ____________________________________________________________________________

#ifdef __TESTING_MODE__
  friend class SurfPointUnitTest;
public:
  static int constructCount;
  static int copyCount;
  static int destructCount;
#endif

};

/// Write point to an output stream in text format
std::ostream& operator<<(std::ostream& os, const SurfPoint& sp); 

#endif

#include "config.h"

#ifndef __SURF_DATA_H__
#define __SURF_DATA_H__

#include <stdexcept>
#include <list>

#include "SurfPoint.h"

class Surface;
class SurfScaler;

/// Contains a set of SurfPoint objects.  May be associated with zero or more
/// Surface objects, which it notifies when its data changes or when it goes 
/// out of existence.  Contains support for exclusion of some of the SurfPoint
/// objects that are physically present, so that clients may operate on some
/// subset of the data if they so choose (e.g., in a cross-validation 
/// algorithm).  Contains methods for I/O support.  Does not allow duplicate
/// points.
/// \todo Allow the points to be weighted differently, as would be needed
/// in weighted regression.
class SurfData
{
public:
/// Nested exception class. A bad_surf_data exception is thrown whenever a 
/// client attempts to:
/// 1) Add a SurfPoint to the SurfData object that has a different number of
///    dimensions and/or a different number of response values than another
///    SurfPoint already in the set;
/// 2) invoke addResponse(...) when there are not yet any SurfPoints;
/// 3) invoke addResponse(...) with the wrong number of values;
/// 4) invoke addResponse(...) on an object where the logical size of the 
///    data set does not match the phyiscal size (i.e., some of the SurfPoints
///    have been marked for exclusion);
/// 5) write a SurfData object that contains no data to a stream;
/// 6) create a Surface with a SurfData object that does not have enough points. 
class bad_surf_data : public std::runtime_error
{
public:
  bad_surf_data(const std::string& msg = "") : std::runtime_error(msg) {}
};

/// Used in conjunction with the caching of SurfData::xMatrix and 
/// SurfData::yVector values.  If an action is taken that would invalidate
/// either of those values (e.g., a new point added to the data set), then
/// the corresponding value inside this StateConsistency object is set to 
/// false.  The SurfData object then knows that if its xMatrix or yVector
/// value is requested, it must be repopulated.
struct StateConsistency 
{
  /// True if the SurfData::xMatrix value is valid.
  bool xMatrix;
  
  /// True if the SurfData::yVector is valid.
  bool yVector;
  
  /// Both values are initially false and are set to true when the SurfData 
  /// members they correspond to are initially filled.
  StateConsistency() : xMatrix(false), yVector(false) {}
};

// ____________________________________________________________________________
// Creation, Destruction, Initialization 
// ____________________________________________________________________________

public:
  /// Vector of points will be copied and checked for duplicates
  SurfData(const std::vector<SurfPoint>& points_);

  /// Read a set of SurfPoints from a file
  SurfData(const std::string filename);

  /// Read a set of SurfPoints from a std::istream
  SurfData(std::istream& is, bool binary = false);

  /// Make a deep copy of the object 
  SurfData(const SurfData& other); 
  
  /// STL data members' resources automatically deallocated 
  ~SurfData();

  /// Copy only the points which have not been marked for exclusion
  SurfData copyActive();
  
  /// First SurfPoint added will determine the dimensions of the data set 
  SurfData();

private:
  /// Data member initialization that is common to all constructors
  void init();
  
  /// Copy xMatrix and yVector from another SurfData object
  void copyBlockData(const SurfData& other);

  /// Deallocate any memory allocated for xMatrix and/or yVector.
  /// Call delete on the SurfPoint* in the data set.
  void cleanup();

// ____________________________________________________________________________
// Overloaded operators 
// ____________________________________________________________________________
public:
  /// Makes deep copy 
  SurfData& operator=(const SurfData& other);

  /// Makes deep comparison
  bool operator==(const SurfData& other) const;

  /// Makes deep comparison
  bool operator!=(const SurfData& other) const;

  /// Return a const reference to SurfPoint at given index
  const SurfPoint& operator[](unsigned index) const;

// ____________________________________________________________________________
// Queries 
// ____________________________________________________________________________

  /// Return the number of SurfPoints in the data set 
  unsigned size() const;

  /// True if there are no points
  bool empty() const;
  
  /// Return the dimensionality of the SurfPoints 
  unsigned xSize() const;

  /// Return the number of response functions in the data set
  unsigned fSize() const;

  /// Returns true if the data has been scaled
  bool isScaled() const;

  /// Return the set of excluded points (the indices)
  const std::set<unsigned>& getExcludedPoints() const ; 

  /// Get the response value of the (index)th point
  double getResponse(unsigned index) const;

  /// Return defaultIndex
  unsigned getDefaultIndex() const;

  /// Return point domains as a matrix in a contiguous block.  Be careful.
  /// The data should not be changed.
  const double* getXMatrix() const;

  /// Return response values for the default response in a contiguous block.  
  /// Be careful. The data should not be changed.
  const double* getYVector() const;

// ____________________________________________________________________________
// Commands 
// ____________________________________________________________________________

  /// Specify which response value getResponse will return. When a Surface 
  /// object that is associated with the SurfData object operates on the data,
  /// it sets this value so that the response value lookup function will return
  /// the value for the response variable that that particular Surface object
  /// is interested in.  
  void setDefaultIndex(unsigned index); 
  
  /// Set the response value of the (index)th point that corresponds to this
  /// surface
  void setResponse(unsigned index, double value);

  /// Calculates parameters so that the data can be viewed as scaled
  void setScaler(SurfScaler* scaler_);
  
  /// Add a point to the data set. The parameter point will be copied.
  void addPoint(const SurfPoint& sp);

  /// Add a new response variable to each point. 
  /// Return the index of the new variable.
  unsigned addResponse(const std::vector<double>& newValues); 
  
  /// Specify which points should be skipped.  This can be used when only a 
  /// subset of the SurfPoints should be used for some computation.
  void setExcludedPoints(const std::set<unsigned>& excludedPoints);

  /// Inform this object that a Surface wants to be notified when this object
  /// changes
  void addListener(Surface*);
 
  /// Remove the Surface from the list of surfaces that are notified when the
  /// data changes
  void removeListener(Surface*);

  /// For use with copy constructor and assignment operator-- creates a list of
  /// pointers to the points in the data set which is used to check for 
  /// duplication when other points are added in the future
  void buildOrderedPoints();

private:
  /// Maps all indices to themselves in the mapping data member
  void defaultMapping();

  /// Creates a matrix of the domains for all of the points in a contiguous
  /// block of memory, for use in matrix operations
  void validateXMatrix() const;
 
  /// Creates a vector of response values for the default response value in
  /// a contiguous blcok of memory
  void validateYVector() const;
public:
   
// ____________________________________________________________________________
// I/O
// ____________________________________________________________________________

  /// Write a set of SurfPoints to a file
  void write(const std::string& filename) const;

  /// Read a set of SurfPoints from a file
  void read(const std::string& filename);
  
  /// Write the surface in binary format
  void writeBinary(std::ostream& os) const;

  /// Write the surface in text format
  void writeText(std::ostream& os) const ; 

  /// Read the surface in binary format
  void readBinary(std::istream& is); 

  /// Read the surface in text format
  void readText(std::istream& is); 

private:

// ____________________________________________________________________________
// Data members 
// ____________________________________________________________________________

  /// Dimensionality of the space from wich the SurfPoints are drawn
  unsigned xsize;

  /// Number of response variables in the data set 
  unsigned fsize;

  /// Controls how the data is scaled during processing
  SurfScaler* scaler;

  /// The set of points in this data set
  std::vector<SurfPoint*> points; 

  /// The indices of points that are to be excluded in computation. This can
  /// be used in a cross-validation scheme to systematically ignore parts of
  /// data set at different times.  
  std::set<unsigned> excludedPoints;

  /// For mapping the indices in points to the indices returned by operator[].
  /// Normally, mapping[i] is equal to i, but the set of excludedPoints is not
  /// empty, this will not be the case.
  std::vector<unsigned> mapping;

  /// Pointer to the domain of the data points, represented as a contiguous 
  /// block of memory
  mutable double* xMatrix;

  /// Pointer to a set of response values for the data poitns, stored in a
  /// contiguous block of memory
  mutable double* yVector;

  /// The index of the response variable that will be returned by F
  mutable unsigned defaultIndex;

  /// Records whether xMatrix and yVector are valid.  They are not populated
  /// unless/until getXMatrix() or getYVector() is called. 
  mutable StateConsistency valid;

public:
  typedef std::set<SurfPoint*,SurfPoint::SurfPointPtrLessThan> SurfPointSet;

private:
  /// Stores the same set of SurfPoint* that points does, but because it is a 
  /// set, membership tests can be done in O(log n) time.  When combined with
  /// the SurfPointPtrLessThan functor object, it allows a SurfData object to
  /// check all SurfPoints in the data set against all others for duplication.
  /// This can be done in O(n log n) time instead of O(n^2).
  SurfPointSet orderedPoints;

  /// List of pointers to listening/observing Surface objects that need to be 
  /// notified when this object changes
  std::list<Surface*> listeners;

// ____________________________________________________________________________
// Constants 
// ____________________________________________________________________________
public:
  /// Used to send a message through Surface::notify(...) that this object is
  /// going out of existence
  static const int GOING_OUT_OF_EXISTENCE;

  /// Used to send a message through Surface::notify(...) that one or more
  /// SurfPoints have been added or modified
  static const int DATA_MODIFIED;
  
// ____________________________________________________________________________
// Helper methods 
// ____________________________________________________________________________

  /// Notify listening surfaces whenever something of interest happens to this
  /// data set
  void notifyListeners(int msg); 

  /// Returns true if file has .sd extension, false if it has .txt extension. 
  /// Otherwise, an exception is thrown.
  bool testFileExtension(const std::string& filename) const;

// ____________________________________________________________________________
// Testing 
// ____________________________________________________________________________

protected:
  // Throw an exception if there are any mismatches in the number of
  // dimensions or number of response values among points in the data set  
  void sanityCheck() const;

  /// Make sure an index falls within acceptable boundaries
  void checkRangeNumPoints(const std::string& header, unsigned index) const;

  /// Make sure an index falls within acceptable boundaries
  void checkRangeNumResponses(const std::string& header, unsigned index) const;

#ifdef __TESTING_MODE__ 
  friend class SurfDataTest;
  friend class SurfaceTest;
#endif
};

/// Print the SurfData to a stream 
std::ostream& operator<<(std::ostream& os, const SurfData& data);

#endif

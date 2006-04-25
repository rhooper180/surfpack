/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2001, Sandia National Laboratories.

    Surfpack: A Software Library of Multidimensional Surface Fitting Methods

    Surfpack is distributed under the DAKOTA GNU General Public License.
    For more information, see the README file in the top Surfpack directory.
    _______________________________________________________________________ */
#ifndef __SURFACE_H__
#define __SURFACE_H__

#include "surfpack_config.h"
#include "surfpack_system_headers.h"
#include "SurfpackParserArgs.h"

class SurfPoint;
class SurfData;
class SurfScaler;

enum MetricType {
  MT_RELATIVE_MAXIMUM,
  MT_RELATIVE_AVERAGE,
  MT_MINIMUM,
  MT_MAXIMUM,
  MT_SUM,
  MT_MEAN
};

/// Abstract base class for implementation of a surface-fitting algorithm.  
/// Each algorithm produces a function approximation given a set of data.  Some
/// algorithms may have options that can be configured by the user.  Each 
/// concrete Surface child class must implement the following methods:
/// 1) Surface* makeSimilarWithNewData(SurfData* sd_);
/// 2) const std::string surfaceName() const;
/// 3) unsigned minPointsRequired() const;
/// 4) double evaluate(const std::vector<double>& x);
/// 5) void build(SurfData& data);
/// 6) void writeBinary(std::ostream& os);
/// 7) void writeText(std::ostream& os);
/// 8) void readBinary(std::ostream& os);
/// 9) void readText(std::ostream& os);
/// \todo One way to allow for "weighting" of data points is to allow one of
/// the response variables in the data set to serve as the weights.  Add a data
/// member to the Surface class that stores the index to the column of weights.
/// (It should work the way the responseIndex member does.) Make it a 
/// configurable argument, so that a user can include an argument of the form
/// weights = 2 in their CreateSurface commands.  That change should be made in
/// Surface::config.  A setter method (e.g., setWeightIndex) is also needed, so
/// that clients can use the option from the C++ API without using the 
/// interpreter front end.
class Surface
{
// ____________________________________________________________________________
// Creation, Destruction, Initialization 
// ____________________________________________________________________________

protected: 
  /// Initialize sd to null
  Surface(); 

public:
  /// Data to be used to create surface is specified 
  Surface(SurfData* sd_);

  /// Deep copies are made of most data members.  For the sd member, the new
  /// object simply asks to be added to other.sd's list of listeners. 
  Surface(const Surface& other);
  
  /// Notifies its SurfData object that it is no longer observing 
  virtual ~Surface(); 

  /// Common initialization for new objects 
  void init();

  /// Create a surface of the same type as 'this.'  This object's data should
  /// be replaced with the data passed in, but all other attributes should
  /// be the same (e.g., a second-order polynomial should return another 
  /// second-order polynomial).  Surfaces returned by this method can be used
  /// to compute the PRESS statistic
  virtual Surface* makeSimilarWithNewData(SurfData* sd_) = 0;

// ____________________________________________________________________________
// Queries 
// ____________________________________________________________________________

public:
  /// Return the name of this surface type 
  virtual const std::string surfaceName() const = 0;
  
  /// Return dimensionality of the surface or zero if not built
  virtual unsigned xSize() const;

  /// Return true if the data that was used to create the surface is available.
  /// Some error metrics require the original data.
  bool hasOriginalData() const;

  /// Return true if there is a sufficient number of correctly formatted data
  /// points
  bool acceptableData() const;

  /// Return the minumum number of points needed to create a surface of this
  /// type. 
  virtual unsigned minPointsRequired() const = 0;
 
  /// Evaluate the approximation surface at point x and return the value.
  /// The point x must have the same dimensionality as this Surface's SurfData.
  virtual double evaluate(const std::vector<double>& x) = 0; 

  /// Evaluate the approximation surface at point x and return the value.
  /// The point x must have the same dimensionality as this Surface's SurfData.
  /// Makes sure the Surface is valid and then call evaluate.
  virtual double getValue(const std::vector<double>& x); 

  /// Evaluate the approximation surface at point x and return the value.
  /// The point x must have the same dimensionality as this surface's SurfData.
  virtual double getValue(const SurfPoint& sp); 

  /// Evaluate the approximation surface at each point in the parameter
  /// surfData object.  Append the evaluations as a new response variable in
  /// the data set.
  virtual void getValue(SurfData& surfData);

  virtual void gradient(const std::vector<double> & x, 
    std::vector<double>& gradient_vector);

  virtual void hessian(const std::vector<double> & x, 
    SurfpackMatrix<double>& hessian);

  /// Evaluate the approximation surface at each point in the parameter
  /// SurfData object.  In the ErrorStruct list, store the expected value (as
  /// returned by sd.getResponse()) and the estimated value.
  virtual void getValue(SurfData& surf_data, 
    std::vector<surfpack::ErrorStruct>& pts);
  
  /// Evaluate the approximation surface at each point in the parameter
  /// SurfData object. Return lists of the observed values (retrieved via
  /// sd.getResponse() for each point sd) and corresponding predicted values 
  /// returned by sd.getResponse()) and the estimated value.  These lists are
  /// returned through the second and third parameters.
  virtual void getValue(SurfData& surf_data, std::vector<double>& observed_vals,
    std::vector<double>& predicted_vals);

  /// Return the value of some error metric
  virtual double goodnessOfFit(const std::string metricName, 
    SurfData* surfData);
  
  /// For each point x in dataSet, construct a similar approximation Surface 
  /// that includes all of the points in dataSet except x.  Then evaluate the
  /// Surface at x.  The difference between x and the estimate of x given the
  /// rest of the data is the residual for x.  The PRESS statistic is the 
  /// square root of the mean of the squares of all the residuals.
  virtual double press(SurfData& dataSet);

  /// Statistically speaking, R^2 is extra sum of squares divided by the total
  /// sum of squares.  It measures how much of the variation in the data is 
  /// accounted for by the model (the approximating surface).
  virtual double rSquared(SurfData& dataSet);

  /// Compute one of several goodness of fit metrics.  The observed parameter
  /// should be a list of observed (or true) function values; the vector of
  /// predicted values gives the corresponding estimates from this surface.
  /// The dt parameter specifies the kind of residuals to compute.  ABSOLUTE
  /// residuals are (observed - predicted), SQUARED residuals are the squares 
  /// of the absolute residuals.  SCALED residuals are the ABSOLUTE residuals
  /// divided by the observed value.  Given the type of residuals, the client
  /// may request the min, max, sum, or mean of the set of residuals over all
  /// the given data points.  Two additional metrics are possible.  The
  /// relative maximum absolute error is the maximum absolute error divided
  /// by the standard deviation of the observed data.  The relative average
  /// absolute error is the mean absolute error divided by the standard 
  /// deviation of observed data. 
  virtual double genericMetric(std::vector<double>& observed,
    std::vector<double>& predicted, enum MetricType mt, enum DifferenceType dt);
  
  /// Return the square root of the mean-squared-error
  double rootMeanSquared(std::vector<double>& observed,
    std::vector<double>& predicted);
// ____________________________________________________________________________
// Commands
// ____________________________________________________________________________

  /// Associates a data set with this Surface object.  When Surface::build()
  /// is invoked, this is the data that will be used to create the Surface
  /// approximation.
  virtual void setData(SurfData* sd_);

  /// Causes the data to be scaled along each dimension so that all of the
  /// values lie on the interval [0,1].  scaled_val = (old_val - min_val) /
  /// (max_val - min_val).
  virtual void scaleUniform();

  /// Causes data not to be scaled at all before building
  virtual void noScale();

  /// Ensures scaler and sd are non-null and forwards Arg on
  void scalingArg(const Arg& arg);

  /// Set the state of the SurfData object to use the same defaultIndex and 
  /// set of excludedPoints that were used when the Surface approximation was
  /// built
  virtual void prepareData();

  /// Return the data set pointed to by member sd if parameter dataSet is NULL.
  /// If dataSet is not NULL, then return the SurfData it points to.  If both
  /// the parameter dataSet and member sd are NULL, throw an exception.  This
  /// method is primarily used with the fitness metrics that can be set to use
  /// by default the data that the approximation was created with but can also
  /// use another set of data provided by the client

  /// Short summary
  virtual SurfData& checkData(SurfData* dataSet);

  /// Invoked by data member sd when the data changes 
  virtual void notify(int msg);

  /// Check to make sure that data are acceptable and then build.
  /// Do not build if the surface has already been built and the data have not
  /// changed.
  virtual void createModel(SurfData* surfData = 0);

  /// Create a surface approximation
  virtual void build(SurfData& data) = 0; 

  /// Modify one of the configuration options for this surface type 
  virtual void config(const Arg& arg);

  /// Process a list of configuration options
  virtual void configList(const ArgList& arglist);

  /// Sets the number of dimensions in the surface
  void setXSize(unsigned xsize_in);

// ____________________________________________________________________________
// I/O 
// ____________________________________________________________________________
public:
  /// Write the surface out to a file.  Files with extension .sps are written
  /// out in text mode; .bsps files are written in binary format (unformatted). 
  virtual void write(const std::string filename);

  /// Read the surface from a file.  Files with extension .sps are read in text 
  /// mode; .bsps are read in binary format (unformatted).
  virtual void read(const std::string filename);

  /// Write the surface in binary format
  virtual void writeBinary(std::ostream& os) = 0; 

  /// Write the surface in text format
  virtual void writeText(std::ostream& os) = 0; 

  /// Read the surface in binary format
  virtual void readBinary(std::istream& is) = 0; 

  /// Read the surface in text format
  virtual void readText(std::istream& is) = 0; 

  /// Return true if filename has .bsps extension, false if filename has .sps
  /// extension.  If neither, throw surfpack::io_exception.
  bool Surface::hasBinaryFileExtension(const std::string& filename) const;


// ____________________________________________________________________________
// Data members 
// ____________________________________________________________________________

protected: 

  /// Data used (or to be used) to create the approximation 
  SurfData* sd;

  /// Necessary if data are to be scaled before calculating surface.
  /// The Surface object always owns (and deletes when it is destroyed) its own
  /// SurfScaler object, so they should not be shared among Surfaces.
  SurfScaler* scaler;

  /// Number of dimensions in the data (sd)
  unsigned xsize;

  /// True if Surface has been successfully built.  This does not imply
  /// that the surface matches the current data, only that there is something
  /// valid to evaluate
  bool builtOK;

  /// True if one or more data points have been modified or added since the 
  /// surface was built
  bool dataModified;

  /// Indices of points present in sd that were not used to make compute the
  /// approximation 
  std::set<unsigned> excludedPoints;

  /// Index of the response in sd that was used to create this surface
  unsigned responseIndex;

// ____________________________________________________________________________
// Testing
// ____________________________________________________________________________

#ifdef __TESTING_MODE__
  friend class SurfaceFactoryUnitTest;
  friend class SurfaceTest;
  friend class PolynomialSurfaceTest;
#endif
};

// Write the surface out to a stream in text format
std::ostream& operator<<(std::ostream& os,Surface&);
#endif

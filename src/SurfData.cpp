#include "surfpack_config.h"
#include "surfpack.h"
#include "SurfData.h"
#include "Surface.h"
#include "SurfScaler.h"

using namespace std;

// ____________________________________________________________________________
// Constants 
// ____________________________________________________________________________

/// Used to send a message through Surface::notify(...) that this object is
/// going out of existence
const int SurfData::GOING_OUT_OF_EXISTENCE = 1;

/// Used to send a message through Surface::notify(...) that one or more
/// SurfPoints have been added or modified
const int SurfData::DATA_MODIFIED = 2;

// ____________________________________________________________________________
// Creation, Destruction, Initialization 
// ____________________________________________________________________________

/// Vector of points will be copied and checked for duplicates
SurfData::SurfData(const vector<SurfPoint>& points_) 
  : scaler(0), valid()
{
  if (points_.empty()) {
    this->xsize = 0;
    this->fsize = 0;
  } else {
    this->xsize = points_[0].xSize();
    this->fsize = points_[0].fSize();
    defaultLabels();
    for (unsigned i = 0; i < points_.size(); i++) {
      this->addPoint(points_[i]);
    }
  }
  init();
  // Check to make sure data points all have the same number of dimensions
  // and response values.  An exception will be thrown otherwise.
  sanityCheck();
}

/// Read a set of SurfPoints from a file
SurfData::SurfData(const string filename) 
: scaler(0), valid() 
{
  init();
  read(filename);
}
  
/// Read a set of SurfPoints from a std::istream
SurfData::SurfData(std::istream& is, bool binary) 
  : scaler(0), valid()
{
  init();
  if (binary) {
    readBinary(is);
  } else {
    readText(is);
  }
}

/// Makes a deep copy of the object 
SurfData::SurfData(const SurfData& other) 
  : xsize(other.xsize), fsize(other.fsize),scaler(other.scaler), 
  excludedPoints(other.excludedPoints), defaultIndex(other.defaultIndex),
  xLabels(other.xLabels), fLabels(other.fLabels)
{
  for (unsigned i = 0; i < other.points.size(); i++) {
    this->addPoint(*other.points[i]);
  }
  copyBlockData(other);
  valid = other.valid;
  mapping = other.mapping;
  buildOrderedPoints();
}

/// First SurfPoint added will determine the dimensions of the data set 
SurfData::SurfData() : scaler(0), valid()
{
    this->xsize = 0;
    this->fsize = 0;
    init();
}

/// STL data members' resources automatically deallocated 
SurfData::~SurfData() 
{
  notifyListeners(GOING_OUT_OF_EXISTENCE);
  listeners.clear();
  cleanup();
}

/// Data member initialization that is common to all constructors
void SurfData::init()
{
  defaultIndex = 0;
  defaultMapping();
  xMatrix = 0;
  yVector = 0;
}

/// Copy only the points which have not been marked for exclusion
SurfData SurfData::copyActive()
{
  // This is not a very efficient method.  It is not expected that it will
  // be called often.  The active points are copied into the local
  // activePoints vector.  Then, a new SurfData object is created, which 
  // requires that all the points be recopied.  Then, at the end of the method
  // the new SurfData object is returned by value, which means all of the data
  // are most likely copied again.  If this is a bottleneck, it would not be
  // terribly difficult to eliminate at least two of the three copies.
  vector<SurfPoint> activePoints;
  for (unsigned i = 0; i < mapping.size(); i++) {
    activePoints.push_back(*points[mapping[i]]);
  }
  SurfData newSD(activePoints);
  if (!activePoints.empty()) {
    newSD.setDefaultIndex(defaultIndex);
  }
  return newSD;
}
  
// Copy xMatrix and yVector from another SurfData object
/// \todo Address the possibility of a memory allocation failure 
void SurfData::copyBlockData(const SurfData& other)
{
  if (other.valid.xMatrix) {
    unsigned numElements = mapping.size() * xsize;
    xMatrix = new double[numElements];
    memcpy(xMatrix,other.xMatrix, numElements*sizeof(double));
  } else {
    xMatrix = 0;
  }

  if (other.valid.yVector) {
    unsigned numElements = mapping.size();
    yVector = new double[numElements];
    memcpy(yVector,other.yVector, numElements*sizeof(double));
  } else {
    yVector = 0;
  }
}
  
/// Deallocate any memory allocated for xMatrix and/or yVector.
/// Call delete on the SurfPoint* in the data set.
void SurfData::cleanup()
{
  delete xMatrix;
  delete yVector;
  xMatrix=0;
  yVector=0;
  valid.xMatrix = false;
  valid.yVector = false;
  mapping.clear();
  orderedPoints.clear();
  for (unsigned j = 0; j < points.size(); j++) {
    delete points[j];
    points[j] =0;
  }
  points.clear();
  excludedPoints.clear();
}

// ____________________________________________________________________________
// Overloaded operators 
// ____________________________________________________________________________

/// Makes deep copy of other
SurfData& SurfData::operator=(const SurfData& other)
{
  if (*this != other) {
    xLabels = other.xLabels;
    fLabels = other.fLabels;
    cleanup();
    this->xsize = other.xsize;
    this->fsize = other.fsize;
    for (unsigned i = 0; i < other.points.size(); i++) {
      this->addPoint(*other.points[i]);
    }
    this->excludedPoints = other.excludedPoints;
    this->mapping = other.mapping;
    this->valid = other.valid;
    this->defaultIndex = other.defaultIndex;
    copyBlockData(other);
  }
  buildOrderedPoints();
  return (*this);
}

/// Makes deep comparison
bool SurfData::operator==(const SurfData& other) const
{
  if (this->xsize == other.xsize && 
      this->fsize == other.fsize &&
      this->size() == other.size()) {
    for (unsigned i = 0; i < points.size(); i++) {
      if (*this->points[i] != *other.points[i]) {
        return false;
      }
    }
    return true;
  } else {
    return false;
  }
}
      
/// Makes deep comparison
bool SurfData::operator!=(const SurfData& other) const
{
  return !(*this == other);
}

/// Return a const reference to SurfPoint at given index
const SurfPoint& SurfData::operator[](unsigned index) const
{
  static string header("Indexing error in SurfData::operator[] const.");
  checkRangeNumPoints(header, index);
  return *points[mapping[index]];
}

// ____________________________________________________________________________
// Queries 
// ____________________________________________________________________________

/// Return the number of SurfPoints in the data set 
unsigned SurfData::size() const 
{ 
  return mapping.size(); 
}

/// True if there are no points
bool SurfData::empty() const
{
  return mapping.empty();
}

/// Return the dimensionality of the SurfPoints 
unsigned SurfData::xSize() const 
{ 
  return xsize; 
}

/// Return the number of response variables in the data set
unsigned SurfData::fSize() const 
{ 
  return fsize; 
}

/// Returns true if the data has been scaled
bool SurfData::isScaled() const
{
  return scaler != 0;
}

/// Return the set of excluded points (the indices)
const std::set<unsigned>& SurfData::getExcludedPoints() const 
{
  return excludedPoints;
}

/// Get the response value of the (index)th point that corresponds to this
/// surface
double SurfData::getResponse(unsigned index) const
{
  static string header("Indexing error in SurfData::getResponse.");
  checkRangeNumPoints(header, index);
  return points[mapping[index]]->F(defaultIndex);
}

/// Get default index
unsigned SurfData::getDefaultIndex() const
{
  return defaultIndex;
}

/// Return point domains as a matrix in a contiguous block.  Be careful.
/// The data should not be changed.
const double* SurfData::getXMatrix() const
{
  if (!valid.xMatrix) {
    validateXMatrix();
  }
  return xMatrix;
}

/// Return response values for the default response in a contiguous block.  
/// Be careful. The data should not be changed.
const double* SurfData::getYVector() const
{
  if (!valid.yVector) {
    validateYVector();
  }
  return yVector;
}
  
/// Retrieve the label for one of the predictor variables
const std::string& SurfData::getXLabel(unsigned index) const
{
  return xLabels[index];
}

/// Retrieve the label for one of the predictor variables
const std::string& SurfData::getFLabel(unsigned index) const
{
  return fLabels[index];
}

/// Retrieve the index and variable type (predictor/response) for a given
/// name.  Return false if not found
bool SurfData::varIndex(const std::string& name, unsigned& index, 
  bool& isResponse) const
{
  vector< string>::const_iterator iter = 
    find(xLabels.begin(),xLabels.end(),name);
  if (iter != xLabels.end()) {
    index = iter - xLabels.begin();
    isResponse = false;
    return true;
  } else {
    iter = find(fLabels.begin(),fLabels.end(),name);
    if (iter != fLabels.end()) {
      index = iter - fLabels.begin();
      isResponse = true;
      return true;
    } else {
      cout << "Name sought: " << name << endl;
      cout << "Predictors: " << endl;
      copy(xLabels.begin(),xLabels.end(),ostream_iterator<string>(cout,"\n"));
      cout << "Responses: " << endl;
      copy(fLabels.begin(),fLabels.end(),ostream_iterator<string>(cout,"\n"));
      return false;
    }
  }
}

// ____________________________________________________________________________
// Commands 
// ____________________________________________________________________________

/// Specify which response value getResponse will return. When a Surface 
/// object that is associated with the SurfData object operates on the data,
/// it sets this value so that the response value lookup function will return
/// the value for the response variable that that particular Surface object
/// is interested in.  
void SurfData::setDefaultIndex(unsigned index)
{
  static string header("Indexing error in SurfData::setDefaultIndex.");
  checkRangeNumResponses(header, index);
  valid.yVector = false;
  defaultIndex = index;
}
  
/// Set the response value of the (index)th point that corresponds to this
/// surface
void SurfData::setResponse(unsigned index, double value)
{
  static string header("Indexing error in SurfData::setResponse.");
  checkRangeNumPoints(header, index);
  points[mapping[index]]->F(defaultIndex, value);
  valid.yVector = false;
}
  
/// Calculates parameters so that the data can be viewed as scaled
void SurfData::setScaler(SurfScaler* scaler_in)
{
  this->scaler = scaler_in;
  if (scaler) {
    assert(scaler->xSize() == xsize);
    assert(scaler->fSize() == fsize);
    enableScaling();
  } else {
    disableScaling();
  }
}
    
/// Add a point to the data set. The parameter point will be copied.
void SurfData::addPoint(const SurfPoint& sp) 
{
  if (points.empty()) {
    xsize = sp.xSize();
    fsize = sp.fSize();
    if (xLabels.empty()) {
      defaultLabels();
    }
  } else {
    if (sp.xSize() != xsize || sp.fSize() != fsize) {
      ostringstream errormsg;
      errormsg << "Error in SurfData::addPoint.  Points in this data set "
	       << "have " << xsize << " dimensions and " << fsize
	       << " response values; point to be added has "
	       << sp.xSize() << " dimensions and " << sp.fSize()
	       << " response values." << endl;
      throw bad_surf_data(errormsg.str());
    }
  }
  SurfPointSet::iterator iter;
  // This should be a safe const cast.  All that's happening is a check
  // to see if another data point at the same location in the space has already
  // been added.
  iter = orderedPoints.find(const_cast<SurfPoint*>(&sp));
  if (iter == orderedPoints.end()) {
    // This SurfPoint is not already in the data set.  Add it.
    points.push_back(new SurfPoint(sp));
    orderedPoints.insert(points[points.size()-1]);
    mapping.push_back(points.size()-1);
  } else {
    // Another SurfPoint in this SurfData object has the same location and
    // may have different response value(s).  Replace the old point with 
    // this new one.
    SurfPoint* spPtr = *iter;
    *spPtr = sp;
  }
  // Since a SurfPoint has been either added or modified, this->xMatrix and
  // this->yVector are no longer valid.
  valid.xMatrix = false;
  valid.yVector = false;
  notifyListeners(SurfData::DATA_MODIFIED);
}

/// Add a new response variable to each point. Return the index of the new 
/// variable.
unsigned SurfData::addResponse(const vector<double>& newValues, 
  std::string label)
{
  unsigned new_index;
  ostringstream errormsg;
  if (points.empty()) {
    throw bad_surf_data(
             "Cannot add response because there are no data points"
          );
  } else if (points.size() != mapping.size()) {
    errormsg << "Cannot add response because physical set size is different "
	     << "than logical set size.\nBefore adding another response, "
             << "clear \"excluded points\" or create a new data set by using " 
	     << "the SurfData::copyActive method." << endl;
    throw bad_surf_data(errormsg.str());
  } else if (newValues.size() != points.size()) {
    errormsg << "Cannot add another response: the number of new response"
             << " values does not match the size of the physical data set." 
             << endl;
    throw bad_surf_data(errormsg.str());
  } else {
    new_index = points[mapping[0]]->addResponse(newValues[0]);
    fsize++;
    for (unsigned i = 1; i < points.size(); i++) {
      new_index = points[mapping[i]]->addResponse(newValues[i]);
      assert(new_index == fsize - 1);
    }
  }
  if (label != "") {
    fLabels.push_back(label);
  } else {
    ostringstream labelos;
    labelos << "'f" << new_index << "'";
    fLabels.push_back(labelos.str());
  }
  return new_index;
}

/// Specify which points should be skipped.  This can be used when only a 
/// subset of the SurfPoints should be used for some computation.
void SurfData::setExcludedPoints(const std::set<unsigned>& excluded_points)
{
  if (excluded_points.size() > points.size()) {
    throw bad_surf_data(
      "Size of set of excluded points exceeds size of SurfPoint set"
    );
  } else if (excluded_points.empty()) {
    defaultMapping();
    this->excludedPoints.clear();
  } else {
    // The size of the logical data set is the size of the physical
    // data set less the number of excluded points    
    mapping.resize(points.size() - excluded_points.size());
    unsigned mappingIndex = 0;
    unsigned sdIndex = 0;
    // map the valid indices to the physical points in points
    while (sdIndex < points.size()) {
      if (excluded_points.find(sdIndex) == excluded_points.end()) {
        mapping[mappingIndex++] = sdIndex;
      }
      sdIndex++;
    }
    this->excludedPoints = excluded_points;
    assert(mappingIndex == mapping.size());
  }
}

/// Inform this object that a Surface wants to be notified when this object
/// changes
void SurfData::addListener(Surface* surface)
{
  /// only add the listener if its not already there
  list<Surface*>::iterator itr = 
    find(listeners.begin(),listeners.end(),surface);
  if(itr ==listeners.end() ) {
    listeners.push_back(surface);
  }
}

/// Remove the Surface from the list of surfaces that are notified when the
/// data changes
void SurfData::removeListener(Surface* surface)
{
  listeners.remove(surface);
}

/// For use with copy constructor and assignment operator-- creates a list of
/// pointers to the points in the data set which is used to check for 
/// duplication when other points are added in the future
void SurfData::buildOrderedPoints()
{
  orderedPoints.clear();
  for (unsigned i = 0; i < points.size(); i++) {
    orderedPoints.insert(points[i]);
  }
}

/// Iterate through the points, setting the current scaler
void SurfData::enableScaling()
{
  for (unsigned i = 0; i < points.size(); i++) {
    points[i]->setScaler(scaler);
  }
}

/// Iterate through the points, clearing the scaler
void SurfData::disableScaling()
{
  for (unsigned i = 0; i < points.size(); i++) {
    points[i]->setScaler(0);
  }
}

/// Maps all indices to themselves in the mapping data member
void SurfData::defaultMapping()
{
  mapping.resize(points.size());
  for (unsigned i = 0; i < points.size(); i++) {
    mapping[i] = i;
  }
}
   
/// Creates a matrix of the domains for all of the points in a contiguous
/// block of memory, for use in matrix operations
void SurfData::validateXMatrix() const
{
  delete xMatrix;
  xMatrix = new double[mapping.size() * xsize];
  for (unsigned pt = 0; pt < mapping.size(); pt++) {
    for (unsigned xval = 0; xval < xsize; xval++) {
      xMatrix[pt+xval*mapping.size()] = points[mapping[pt]]->X()[xval];
    }
  }
  valid.xMatrix = true;
}

/// Creates a vector of response values for the default response value in
/// a contiguous blcok of memory
void SurfData::validateYVector() const
{
  delete yVector;
  yVector = new double[mapping.size()];
  for (unsigned pt = 0; pt < mapping.size(); pt++) {
    yVector[pt] = getResponse(pt);
  }
  valid.yVector = true;
}

/// Set the labels for the predictor variables
void SurfData::setXLabels(std::vector<std::string>& labels)
{
  if (labels.size() != xsize) {
    throw string("Dim mismatch in SurfData::setXLabels");
  }
  xLabels = labels;
}

/// Set the labels for the response variables
void SurfData::setFLabels(std::vector<std::string>& labels)
{
  if (labels.size() != xsize) {
    throw string("Dim mismatch in SurfData::setFLabels");
  }
  fLabels = labels;

}

// ____________________________________________________________________________
// I/O
// ____________________________________________________________________________

/// Write a set of SurfPoints to a file.  Opens the file and calls other 
/// version of write.
void SurfData::write(const std::string& filename) const
{
  if (mapping.empty()) {
    ostringstream errormsg;
    errormsg << "Cannot write SurfData object to stream."
	     << "  No active data points." << endl;
    throw bad_surf_data(errormsg.str());
  }
  bool binary = testFileExtension(filename);
  ofstream outfile(filename.c_str(), 
    (binary ? ios::out|ios::binary : ios::out));
  if (!outfile) {
    throw surfpack::file_open_failure(filename);
  } else if (binary) {
    writeBinary(outfile);
  } else {
    writeText(outfile);
  }
  outfile.close();
}

/// Read a set of SurfPoints from a file.  Opens file and calls other version.
void SurfData::read(const string& filename)
{
  // Open file in binary or text mode based on filename extension (.sd or .txt)
  bool binary = testFileExtension(filename);
  ifstream infile(filename.c_str(), (binary ? ios::in|ios::binary : ios::in));
  if (!infile) {
    throw surfpack::file_open_failure(filename);
  } else if (binary) {
    readBinary(infile);
  } else {
    readText(infile);
  }
  // Object may have already been created
  valid.xMatrix = false;
  valid.yVector = false;
  infile.close();
}

/// Write a set of SurfPoints to an output stream
void SurfData::writeBinary(ostream& os) const 
{
  unsigned s = mapping.size();
  os.write((char*)&s,sizeof(s));
  os.write((char*)&xsize,sizeof(xsize));
  os.write((char*)&fsize,sizeof(fsize));
  for (unsigned i = 0; i < mapping.size(); i++) {
    points[mapping[i]]->writeBinary(os);
  }
}

/// Write a set of SurfPoints to an output stream
void SurfData::writeText(ostream& os) const
{
    os << mapping.size() << endl
       << xsize << endl 
       << fsize << endl;
    for (unsigned i = 0; i < xLabels.size(); i++) {
      os << setw(surfpack::field_width) << xLabels[i];
    }
    for (unsigned i = 0; i < fLabels.size(); i++) {
      os << setw(surfpack::field_width) << fLabels[i];
    }
    os << endl;
    for (unsigned i = 0; i < mapping.size(); i++) {
      points[mapping[i]]->writeText(os);
    }
}

/// Read a set of SurfPoints from an input stream
void SurfData::readBinary(istream& is) 
{
  unsigned numPointsRead = 0;
  unsigned size;
  try {
    cleanup();
    is.read((char*)&size,sizeof(size));
    is.read((char*)&xsize,sizeof(xsize));
    is.read((char*)&fsize,sizeof(fsize));
    points.clear();
    for (numPointsRead = 0; numPointsRead < size; numPointsRead++) {
      // Throw an exception if we hit the end-of-file before we've
      // read the number of points that were supposed to be there.
      surfpack::checkForEOF(is);
      // True for fourth argument signals a binary read
      this->addPoint(SurfPoint(xsize,fsize,is,true));  
    }
    defaultMapping();
  } catch(surfpack::io_exception&) {
    cerr << "Expected: " << size << " points.  "
         << "Read: " << numPointsRead << " points." << endl;
    throw;
  } 
}

/// Read a set of SurfPoints from an input stream
void SurfData::readText(istream& is) 
{
  unsigned numPointsRead = 0;
  unsigned size;
  try {
    cleanup();
    string sline;
    getline(is,sline);
    istringstream streamline(sline);
    streamline >> size;
    getline(is,sline);
    streamline.str(sline);
    streamline.clear();
    streamline >> xsize;
    getline(is,sline);
    streamline.str(sline);
    streamline.clear();
    streamline >> fsize;
    points.clear();

    getline(is,sline);
    streamline.str(sline);
    streamline.clear();
    if (!readLabelsIfPresent(streamline)) {
      defaultLabels();
      if (sline != "" && sline != "\n") {
        istringstream firstpoint(sline);
        this->addPoint(SurfPoint(xsize,fsize,firstpoint,false));
        numPointsRead = 1;
      }
    }
    while (numPointsRead < size) {
      // Throw an exception if we hit the end-of-file before we've
      // read the number of points that were supposed to be there.
      surfpack::checkForEOF(is);
      // False for last argument signals a text read
      this->addPoint(SurfPoint(xsize,fsize,is,false));  
      numPointsRead++;
    }
    defaultMapping();
  } catch(surfpack::io_exception& ioException) {
    cerr << ioException.what() << endl
         << "Expected: " << size << " points.  "
         << "Read: " << numPointsRead << " points." << endl;
    throw;
  } 
}

// Print set of data points to a stream. 
ostream& operator<<(ostream& os, const SurfData& sd) 
{ 
  sd.writeText(os); 
  return os;
}

// ____________________________________________________________________________
// Helper methods 
// ____________________________________________________________________________

/// Returns true if file has .sd extension, false if it has .txt extension. 
/// Otherwise, an exception is thrown.
bool SurfData::testFileExtension(const std::string& filename) const
{
  if (surfpack::hasExtension(filename,".sd")) {
    return true;
  } else if (surfpack::hasExtension(filename,".txt")) {
    return false;
  } else {
    throw surfpack::io_exception(
      "Unrecognized filename extension.  Use .sd or .txt"
    );
  }
}

/// Notify listening surfaces whenever something of interest happens to this
/// data set
void SurfData::notifyListeners(int msg)
{
  if (listeners.size() != 0) {
    // Do nothing
  }
  list<Surface*>::iterator itr = listeners.begin();
  while (itr != listeners.end()) {
    if (*itr) {
      (*itr)->notify(msg);
    }
    ++itr;
  }
}

/// Set x vars labels to 'x0' 'x1', etc.; resp. vars to 'f0' 'f1', etc.
void SurfData::defaultLabels()
{
  xLabels.resize(xsize);
  for (unsigned i = 0; i < xsize; i++) {
    ostringstream os;
    os << "'x" << i << "'";
    xLabels[i] = os.str();
  }
  fLabels.resize(fsize);
  for (unsigned i = 0; i < fsize; i++) {
    ostringstream os;
    os << "'f" << i << "'";
    fLabels[i] = os.str();
  }
}

bool SurfData::readLabelsIfPresent(std::istream& is)
{
  string label;
  xLabels.resize(xsize);
  for (unsigned i = 0; i < xsize; i++) {
    is >> label;
    int first = label.find('\'');
#ifdef HAVE_STD
    if (first == string::npos) {
#else
    if (first <= label.size() || first > 0) {
#endif
      return false;
    }
    xLabels[i] = label;
  }
  fLabels.resize(fsize);
  for (unsigned i = 0; i < fsize; i++) {
    is >> fLabels[i];
  }
  return true;
}
// ____________________________________________________________________________
// Testing 
// ____________________________________________________________________________

// Throw an exception if there are any mismatches in the number of
// dimensions or number of response values among points in the data set  
void SurfData::sanityCheck() const
{
  if (!points.empty()) {
    unsigned dimensionality = points[0]->xSize();
    unsigned numResponses = points[0]->fSize();
    for (unsigned i = 1; i < points.size(); i++) {
      if (points[i]->xSize() != dimensionality ||
          points[i]->fSize() != numResponses ) {
        ostringstream errormsg;
        errormsg << "Error in SurfData::sanityCheck." << endl
		 << "Point 0 has " << dimensionality << " dimensions "
                 << "and " << numResponses << " response values, " << endl
                 << "but point " << i << " has " << points[i]->xSize()
 		 << " dimensions and " << points[i]->fSize() << "response "
 		 << " values.";
	throw bad_surf_data(errormsg.str());
      } // if mismatch
    } // for each point
  } // if !empty
}

/// Check that the index falls within acceptable boundaries (i.e., is
/// less than mapping.size()
void SurfData::checkRangeNumPoints(const string& header, unsigned index) const
{
  if (index >= mapping.size()) {
    ostringstream errormsg;
    errormsg << header << endl;
    if (mapping.empty()) {
      errormsg << "Index " << index << " specified, but there are zero points "
	       << "in the logical data set."
               << endl;
    } else {
      errormsg << "Requested: " 
	     << index 
	     << "; actual max index: "
	     << mapping.size() - 1
	     << endl;
    }
    throw range_error(errormsg.str());
  }
}

/// Make sure an index falls within acceptable boundaries (i.e., index is less
/// than fsize)
void SurfData::checkRangeNumResponses(const string& header, 
  unsigned index) const
{
  if (index >= fsize) {
    ostringstream errormsg;
    errormsg << header << endl;
    if (fsize == 0) {
      errormsg << "Index " << index 
	       << " specified, but there are zero response"
	       << "values."
               << endl;
    } else {
      errormsg << "Requested: " 
	     << index 
	     << "; actual max index: "
	     << fsize - 1
	     << endl;
    }
    throw range_error(errormsg.str());
  }
}

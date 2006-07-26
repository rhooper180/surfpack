/*  _______________________________________________________________________
 
    Surfpack: A Software Library of Multidimensional Surface Fitting Methods
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU General Public License.
    For more information, see the README file in the top Surfpack directory.
    _______________________________________________________________________ */

#include "surfpack_config.h"
#include "surfpack.h"
#include "SurfData.h"
#include "KrigingSurface.h"

//#define PRINT_DEBUG

using namespace std;
using namespace surfpack;
const int dbgkrig = 0;

const string KrigingSurface::name = "Kriging";
//_____________________________________________________________________________
// Creation, Destruction, Initialization
//_____________________________________________________________________________

KrigingSurface::KrigingSurface(SurfData* sd)
  : Surface(sd), needsCleanup(false), runConminFlag(true)
{
#ifdef __TESTING_MODE__
  constructCount++;
#endif
  xsize = numsamp = 0;
}

KrigingSurface::KrigingSurface(const string filename) : Surface(0)
{
#ifdef __TESTING_MODE__
  constructCount++;
#endif
  read(filename);
}

KrigingSurface::~KrigingSurface() 
{
#ifdef __TESTING_MODE__
  destructCount++;
#endif
   if (needsCleanup) {
      cleanup();
   }
}

void KrigingSurface::initialize()
{
  if (xsize <= 0 || numsamp <= 0) {
    cerr << "Trying to create Kriging Surface with bad or non-existent surfdata" << endl;
  }
  // CONMIN parameters and array allocation
  NFDG   = 0;       // default finite difference flag
  IPRINT = dbgkrig;       // default flag to control amount of output info
  ITMAX  = 100;     // default max number of iterations
  FDCH   =  1.0e-5; // default relative finite difference step size
  FDCHM  =  1.0e-5; // default absolute finite difference step size
  CT     = -0.1;    // default constraint "thickness tolerance 
                    // (for determining active/inactive constraint status)
  CTMIN  =  0.001;  // default absolute constraint tolerance 
                    // (note: the CONMIN manual default is 0.004)
  CTL    = -0.01;   // default side constraint thickness tolerance (see CT)
  CTLMIN =  0.001;  // default absolute side constraint tolerance
  DELFUN =  1.0e-7; // default minimum relative change in the objective
                    // function needed for convergence
  DABFUN =  1.0e-7; // default minimum absolute change in the objective
                    // function needed for convergence
  conminInfo = 0;   // must be set to 0 before calling CONMIN
  NSIDE  = 1;       // flag to identify existence of side constraints in 
                    // optimization subproblem to solve for kriging 
                    // correlation parameters

  numcon = 0;     // number of constraints
  N1     = xsize + 2;
  N2     = xsize*2 + numcon;
  N3     = 1 + numcon + xsize;
  N4     = (N3 >= xsize) ? N3 : xsize;
  N5     = 2*N4;
  S      = new double[N1];
  G1     = new double[N2];
  G2     = new double[N2];
  B      = new double[N3*N3];
  C      = new double[N4];
  MS1    = new int[N5];
  SCAL   = new double[N1];
  DF     = new double[N1];
  A      = new double[N1*N3];
  ISC    = new int[N2];
  IC     = new int[N3];
  conminThetaVars      = new double[N1];
  conminThetaLowerBnds = new double[N1];  
  conminThetaUpperBnds = new double[N1];
  
  ICNDIR   =  xsize+1;  // conjugate direction restart parameter
  NSCAL    =  0;  // (unused) scaling flag
  NACMX1   =  N3; // estimate of 1+(max # of active constraints)
  LINOBJ   =  0;  // (unused) set to 1 if obj fcn is linear
  ITRM     =  3;  // number of consecutive iterations of less than DELFUN
                  // or DABFUN progress before optimization terminated
  THETA    =  1.0;// mean value of "push-off factor" in mtd of feasible
                  // directions
  PHI      =  5.0;// "participation coefficient" to force an infeasible
                  // design to be feasible
  ALPHAX   = 0.1; // 1-D search fractional change parameter
  ABOBJ1   = 0.1; // 1-D search fractional change parameter for 1st step

  // initialize CONMIN arrays IC and ISC
  int i;
  for( i=0; i<numcon; i++ ) {
    IC[i]  = 0;
    ISC[i] = 0;
  }

  // Kriging parameters and array allocation
  iFlag             = 1;
  betaHat           = 0.0;
  maxLikelihoodEst  = 0.0;
  numNewPts         = 1;
  numSampQuad       = 4*numsamp;
  conminSingleArray = 1;
  xNewVector        = new double[xsize*numNewPts];
  thetaVector       = new double[xsize];
  xArray 	    = new double[xsize];
  xMatrix           = new double[xsize*numsamp];
  yValueVector      = new double[numsamp];
  rhsTermsVector    = new double[numsamp];
  constraintVector  = new double[conminSingleArray];
  thetaLoBndVector  = new double[xsize];
  thetaUpBndVector  = new double[xsize];
  iPivotVector      = new int[numsamp];
  correlationMatrix = new double[numsamp*numsamp];
  invcorrelMatrix   = new double[numsamp*numsamp];
  fValueVector      = new double[numsamp];
  fRinvVector       = new double[numsamp];
  yfbVector         = new double[numsamp];
  yfbRinvVector     = new double[numsamp];
  rXhatVector       = new double[numsamp];
  workVector        = new double[numsamp];
  workVectorQuad    = new double[4*numsamp];
  iworkVector       = new int[numsamp];
  yNewVector        = new double[numNewPts];

  // Initialize 'thetaVector' which is the vector of correlation 
  // parameters for the kriging model -- maximum liklihood estimation
  // is used to find the optimal values of thetaVector in the kriging code.
  // Also initialize the lower and upper bounds on theta.
  /*for (i=0; i<xsize; i++) {
    thetaVector[i]      = theta_input[i];
    thetaLoBndVector[i] = theta_low_bnd_input[i];
    thetaUpBndVector[i] = theta_upp_bnd_input[i];
  }*/

  // copy the "theta" vectors to the "conmin_theta" -- this is needed 
  // because CONMIN uses array sizes that are larger than the number
  // of design variables (array length = N1, and N1 = xsize+2)
  for (i=0;i<N1;i++) {
    conminThetaVars[i]      = 0.0;
    conminThetaLowerBnds[i] = -DBL_MAX;
    conminThetaUpperBnds[i] =  DBL_MAX;
  }
  for (i=0;i<xsize;i++) {
    conminThetaVars[i]      = 1.0;
    conminThetaLowerBnds[i] = 1.e-3; 
    conminThetaUpperBnds[i] = 1.e+6; 
  }

  // now that all this memory has been allocated, we need to set a flag
  // so that the object will know to delete it
  needsCleanup = true;
}

void KrigingSurface::cleanup()
{
  // clean up CONMIN array allocation
  delete [] conminThetaVars; conminThetaVars = 0;
  delete [] conminThetaLowerBnds; conminThetaLowerBnds = 0;
  delete [] conminThetaUpperBnds; conminThetaUpperBnds = 0;
  delete [] S; S = 0;
  delete [] G1; G1 = 0;
  delete [] G2; G2 = 0;
  delete [] B; B = 0;
  delete [] C; C = 0;
  delete [] MS1; MS1 = 0;
  delete [] SCAL; SCAL = 0;
  delete [] DF; DF = 0;
  delete [] A; A = 0;
  delete [] ISC; ISC = 0;
  delete [] IC; IC = 0;

  // clean up kriging array allocation
  delete [] xNewVector ; xNewVector = 0;
  delete [] thetaVector ; thetaVector = 0;
  delete [] xArray; xArray = 0;
  delete [] xMatrix ; xMatrix = 0;
  delete [] yValueVector ; yValueVector = 0;
  delete [] rhsTermsVector ; rhsTermsVector = 0;
  delete [] constraintVector ; constraintVector = 0;
  delete [] thetaLoBndVector; thetaLoBndVector = 0;
  delete [] thetaUpBndVector; thetaUpBndVector = 0;
  delete [] iPivotVector ; iPivotVector = 0;
  delete [] correlationMatrix ; correlationMatrix = 0;
  delete [] invcorrelMatrix ; invcorrelMatrix = 0;
  delete [] fValueVector ; fValueVector = 0;
  delete [] fRinvVector ; fRinvVector = 0;
  delete [] yfbVector ; yfbVector = 0;
  delete [] yfbRinvVector ; yfbRinvVector = 0;
  delete [] rXhatVector ; rXhatVector = 0;
  delete [] workVector ; workVector = 0;
  delete [] workVectorQuad ; workVectorQuad = 0;
  delete [] iworkVector ; iworkVector = 0;
  delete [] yNewVector ; yNewVector = 0;
}

//_____________________________________________________________________________
// Queries 
//_____________________________________________________________________________

const string KrigingSurface::surfaceName() const
{
  return name;
}

unsigned KrigingSurface::minPointsRequired(unsigned hypothetical_xsize) 
{ 
  return hypothetical_xsize + 1;
}

unsigned KrigingSurface::minPointsRequired() const
{ 
  if (this->xsize <= 0) {
    throw string(
      "Dimensionality of data needed to determine number of required samples."
    );
  } else {
    return minPointsRequired(this->xsize);
  }
}

double KrigingSurface::evaluate(const std::vector<double>& x)
{ 
  static int called = 0;
  ++called;
  //double* xArray = new double[xsize];
  if (x.size() != xsize) {
    cerr << "Wrong number of dimensions" << endl;
  } else {
    for (unsigned i = 0; i < xsize; i++) {
      xArray[i] = x[i];
    }
  }
  yNewVector[0] =  0.0;
  iFlag = 2; // value = 2 for evaluating a previously computed kriging model
#ifdef PRINT_DEBUG
  ostream& os = cout;
  os << "Before call to krigmodel in calculate" << endl;
  printKrigModelVariables(os);
#endif
  //cout << "Calculate_________________" << endl;
  //printKrigEvalVars(cout);
  int xsize_as_int = static_cast<int>(xsize);
  KRIGMODEL_F77(xsize_as_int,numsamp,numNewPts,
		iFlag,thetaVector[0],xMatrix[0],yValueVector[0],xArray[0],
		yNewVector[0],betaHat,rhsTermsVector[0],maxLikelihoodEst,
		iPivotVector[0],correlationMatrix[0],invcorrelMatrix[0],
		fValueVector[0],fRinvVector[0],yfbVector[0],yfbRinvVector[0],
		rXhatVector[0],workVector[0],workVectorQuad[0],iworkVector[0],
		numSampQuad);
#ifdef PRINT_DEBUG
  os << "After call to krigmodel in calculate" << endl;
  printKrigModelVariables(os);
#endif
  return yNewVector[0];
}
    
//_____________________________________________________________________________
// Commands 
//_____________________________________________________________________________

void KrigingSurface::setConminThetaVars(const std::vector<double>& vals)
{
  if (sd && sd->xSize() != vals.size()) {
    throw string("Dimension mismatch: conmin seed and data dimensionality");
  } else { 
    numsamp = sd->size();
    if (!needsCleanup) {
      initialize();
    }
    for (unsigned i = 0; i < vals.size(); i++) {
            conminThetaVars[i] = vals[i];
    }
    runConminFlag = true;
  }
}

void KrigingSurface::useUniformCorrelationValue(double correlation)
{
  if (xsize == 0) {
    throw string("Must know data arity to use uniform correlation value.");
  }
  std::vector<double> vals(xsize);
  for (unsigned i = 0; i < xsize; i++) vals[i] = correlation;
  usePreComputedCorrelationVector(vals);
}
void KrigingSurface::
  usePreComputedCorrelationVector(const std::vector<double>& vals)
{
  if (needsCleanup) {
    delete thetaVector;
  }
  if (sd && sd->xSize() != vals.size()) {
    throw string("Dimension mismatch: correlations and data dimensionality");
  }
  xsize = vals.size();
  thetaVector = new double[xsize];
  for (unsigned i = 0; i < vals.size(); i++) {
          thetaVector[i] = vals[i];
  }
  runConminFlag = false;
}

void KrigingSurface::build(SurfData& data)
{
  double* saveTheta = 0;

  // code from Surface::build that needs to be executed before
  // initialize
  // this could be cleaner

  numsamp = data.size();
  
  if (!runConminFlag) {
    saveTheta = new double[xsize];
    memcpy(saveTheta,thetaVector,sizeof(double)*xsize);
  }

  if (needsCleanup) {
    cleanup();
  }
  initialize();

  if (!runConminFlag) {
    memcpy(thetaVector,saveTheta,sizeof(double)*xsize);
    delete saveTheta;
    saveTheta = 0;
  }
  buildModel(data);
}

void KrigingSurface::config(const Arg& arg)
{
  vector<double> temp;
  string argname = arg.name;
  if (argname == "conmin_seed") {
    setConminThetaVars(RvalTuple::asVectorDouble(temp,arg.getRVal()->getTuple())); 
  } else if (argname == "correlations") {
    usePreComputedCorrelationVector(RvalTuple::asVectorDouble(temp,arg.getRVal()->getTuple()));
  } else if (argname == "uniform_correlation") {
    useUniformCorrelationValue(arg.getRVal()->getReal());
  } else {
    Surface::config(arg);
  }
}
/// Create a surface of the same type as 'this.'  This objects data should
/// be replaced with the dataItr passed in, but all other attributes should
/// be the same (e.g., a second-order polynomial should return another 
/// second-order polynomial.  Surfaces returned by this method can be used
/// to compute the PRESS statistic.
KrigingSurface* KrigingSurface::makeSimilarWithNewData(SurfData* surfData)
{
  return new KrigingSurface(surfData);
}
//_____________________________________________________________________________
// I/O 
//_____________________________________________________________________________

void KrigingSurface::writeBinary(ostream& os)
{
  int i;
  unsigned j;
  os.write((char*)(&xsize),sizeof(int));
  os.write((char*)(&numsamp),sizeof(int));
  for(i=0;i<numsamp;i++) { 
    os.write(reinterpret_cast<char*>(&xMatrix[i]),sizeof(xMatrix[i]));
  }
  for(i=0;i<numsamp;i++) { 
    os.write(reinterpret_cast<char*>(&rXhatVector[i]),sizeof(rXhatVector[i]));
  }
  os.write(reinterpret_cast<char*>(&betaHat),sizeof(betaHat));
  for(i=0;i<numsamp;i++) { 
    os.write(reinterpret_cast<char*>(&rhsTermsVector[i]),sizeof(rhsTermsVector[i]));
  }
  for(j=0;j<xsize;j++) { 
    os.write(reinterpret_cast<char*>(&thetaVector[j]),sizeof(thetaVector[j]));
  }
}

void KrigingSurface::writeText(ostream& os)
{
  ios::fmtflags old_flags = os.flags();
  unsigned old_precision = os.precision(surfpack::output_precision);
  os.setf(ios::scientific);
  int i;
  unsigned j;
  os << numsamp << " number of data points" << endl;
  os << xsize << " number of input variables" << endl;
  for(i=0;i<numsamp;i++) { 
    os << xMatrix[i] << " xMatrix[" << i << "]" << endl; 
  }
  for(i=0;i<numsamp;i++) { 
    os <<  rXhatVector[i] << " rXhatVector[" << i << "]" << endl; 
  }
  os << betaHat << " betaHat" << endl; 
  for(i=0;i<numsamp;i++) { 
    os <<  rhsTermsVector[i] << " rhsTermsVector[" << i << "]" << endl; 
  }
  for(j=0;j<xsize;j++) { 
    os << thetaVector[j] << " thetaVector[" << j << "]" << endl; 
  }
  os.flags(old_flags);
  os.precision(old_precision);
}

void KrigingSurface::readBinary(istream& is)
{
  int i;
  unsigned j;
  is.read((char*)(&xsize),sizeof(int));
  is.read((char*)(&numsamp),sizeof(int));
  initialize();
  for(i=0;i<numsamp;i++) { 
    is.read(reinterpret_cast<char*>(&xMatrix[i]),sizeof(xMatrix[i]));
  }
  for(i=0;i<numsamp;i++) { 
    is.read(reinterpret_cast<char*>(&rXhatVector[i]),sizeof(rXhatVector[i]));
  }
  is.read(reinterpret_cast<char*>(&betaHat),sizeof(betaHat));
  for(i=0;i<numsamp;i++) { 
    is.read(reinterpret_cast<char*>(&rhsTermsVector[i]),sizeof(rhsTermsVector[i]));
  }
  for(j=0;j<xsize;j++) { 
    is.read(reinterpret_cast<char*>(&thetaVector[j]),sizeof(thetaVector[j]));
  }
}

void KrigingSurface::readText(istream& is)
{
  string sline;
  istringstream streamline;
  int i;
  unsigned j;
  getline(is,sline); streamline.str(sline);
  streamline >> numsamp;
  getline(is,sline); streamline.str(sline);
  streamline >> xsize;
  initialize();
  for(i=0;i<numsamp;i++) { 
    getline(is,sline); streamline.str(sline);
    streamline >> xMatrix[i];
  }
  for(i=0;i<numsamp;i++) { 
    getline(is,sline); streamline.str(sline);
    streamline >> rXhatVector[i];
  }
  getline(is,sline); streamline.str(sline);
  streamline >> betaHat;
  for(i=0;i<numsamp;i++) { 
    getline(is,sline); streamline.str(sline);
    streamline >> rhsTermsVector[i];
  }
  for(i=0;i<xsize;i++) { 
    getline(is,sline); streamline.str(sline);
    streamline >> thetaVector[i];
  }

}

void KrigingSurface::printKrigModelVariables(ostream& os)
{
  int i;
  unsigned j;
  os << "After call to krigmodel in modelbuild" << endl;
  os << "xsize: " << xsize << endl;
  os << "numsamp: " << numsamp  << endl;
  os << "numNewPts: " << numNewPts << endl;
  os << "iFlag" << iFlag << endl;
  os << "thetaVector" << endl; for(j=0;j<xsize;j++) { os << thetaVector[j] << endl; }
  os << "xMatrix" << endl; for(i=0;i<numsamp;i++) { os << xMatrix[i] << endl; }
  os << "yValueVector" << endl; for(i=0;i<numsamp;i++) { os <<  yValueVector[i] << endl; }
  os << "xNewVector" << endl; for(i=0;i<numsamp;i++) { os <<  xNewVector[i]  << endl; }
  os << "yNewVector" << endl; for(i=0;i<numsamp;i++) { os <<  yNewVector[i] << endl; }
  os << "betaHat: " <<  betaHat << endl; 
  os << "rhsTermsVector" << endl; for(i=0;i<numsamp;i++) { os <<  rhsTermsVector[i] << endl; }
  os << "maxLikelihoodEst: " << maxLikelihoodEst << endl;
  os << "iPivotVector" << endl; for(i=0;i<numsamp;i++) { os <<  iPivotVector[i] << endl; }
  os << "correlationMatrix" << endl; for(i=0;i<numsamp*numsamp;i++) { os <<  correlationMatrix[i] << endl; }
  os << "invcorrelMatrix" << endl; for(i=0;i<numsamp*numsamp;i++) { os <<  invcorrelMatrix[i] << endl; }
  os << "fValueVector" << endl; for(i=0;i<numsamp;i++) { os <<  fValueVector[i] << endl; }
  os << "fRinvVector" << endl; for(i=0;i<numsamp;i++) { os <<  fRinvVector[i] << endl; }
  os << "yfbVector" << endl; for(i=0;i<numsamp;i++) { os <<  yfbVector[i] << endl; }
  os << "yfbRinvVector" << endl; for(i=0;i<numsamp;i++) { os <<  yfbRinvVector[i] << endl; }
  os << "rXhatVector" << endl; for(i=0;i<numsamp;i++) { os <<  rXhatVector[i] << endl; }
  os << "workVector" << endl; for(i=0;i<numsamp;i++) { os <<  workVector[i] << endl; }
  os << "workVectorQuad" << endl; for(i=0;i<4*numsamp;i++) { os <<  workVectorQuad[i] << endl; }
  os << "iworkVector" << endl; for(i=0;i<numsamp;i++) { os <<  iworkVector[i] << endl; }
  os << "numSampQuad: " << numsamp  << endl;
}

void KrigingSurface::printKrigEvalVars(ostream& os)
{
  int i;
  unsigned j;
  os << "xMatrix" << endl; for(i=0;i<numsamp;i++) { os << xMatrix[i] << endl; }
  os << "rXhatVector" << endl; for(i=0;i<numsamp;i++) { os <<  rXhatVector[i] << endl; }
  os << "betaHat: " <<  betaHat << endl; 
  os << "rhsTermsVector" << endl; for(i=0;i<numsamp;i++) { os <<  rhsTermsVector[i] << endl; }
  os << "numsamp: " << numsamp  << endl;
  os << "xsize: " << xsize << endl;
  os << "numNewPts: " << numNewPts << endl;
  os << "thetaVector: " << endl; for(j=0;j<xsize;j++) { os << thetaVector[j] << endl; }
  os << "yNewVector" << endl; for(i=0;i<numsamp;i++) { os <<  yNewVector[i] << endl; }
}

void KrigingSurface::printConminVariables(ostream& os)
{
  int i;
  os << "ConminThetaVars" << endl; for(i=0;i<N1;i++) { os << conminThetaVars[i] << endl; }
  os << "ConminThetaLowerBnds" << endl; for(i=0;i<N1;i++) { os << conminThetaLowerBnds[i] << endl; }
  os << "ConminThetaUpperBnds" << endl; for(i=0;i<N1;i++) { os << conminThetaUpperBnds[i] << endl; }
  os << "ConstraintVector" << endl; for(i=0;i<conminSingleArray;i++) { os << constraintVector[i] << endl; }
  os << "SCAL" << endl; for(i=0;i<N1;i++) { os << SCAL[i] << endl; }
  os << "DF" << endl; for(i=0;i<N1;i++) { os << DF[i] << endl; }
  os << "A" << endl; for(i=0;i<N1*N3;i++) { os << A[i] << endl; }
  os << "S" << endl; for(i=0;i<N1;i++) { os << S[i] << endl; }
  os << "G1" << endl; for(i=0;i<N2;i++) { os << G1[i] << endl; }
  os << "G2" << endl; for(i=0;i<N2;i++) { os << G2[i] << endl; }
  os << "B" << endl; for(i=0;i<N3*N3;i++) { os << B[i] << endl; }
  os << "C" << endl; for(i=0;i<N4;i++) { os << C[i] << endl; }
  os << "ISC" << endl; for(i=0;i<N2;i++) { os << ISC[i] << endl; }
  os << "IC" << endl; for(i=0;i<N3;i++) { os << IC[i] << endl; }
  os << "MS1" << endl; for(i=0;i<N5;i++) { os << MS1[i] << endl; }
  os << "N1: " << N1 << endl;
  os << "N2: " << N2 << endl;
  os << "N3: " << N3 << endl;
  os << "N4: " << N4 << endl;
  os << "N5: " << N5 << endl;
  os << "N5: " << N5 << endl;
  os << "DELFUN: " << DELFUN << endl;
  os << "DABFUN: " << DABFUN << endl;
  os << "FDCH: " <<  FDCH<< endl;
  os << "FDCHM: " <<  FDCHM<< endl;
  os << "CT: " << CT << endl;
  os << "CTMIN: " << CTMIN << endl;
  os << "CTL: " << CTL << endl;
  os << "CTLMIN: " << CTLMIN << endl;
  os << "ALPHAX: " << ALPHAX << endl;
  os << "ABOBJ1: " << ABOBJ1 << endl;
  os << "THETA: " << THETA << endl;
  os << "maxLikelihoodEst: " << maxLikelihoodEst << endl;
  os << "xsize: " << xsize << endl;
  os << "numcon: " << numcon << endl;
  os << "NSIDE: " << NSIDE << endl;
  os << "IPRINT: " << IPRINT  << endl;
  os << "NFDG: " << NFDG  << endl;
  os << "NSCAL: " << NSCAL << endl;
  os << "LINOBJ: " << LINOBJ << endl;
  os << "ITMAX: " << ITMAX  << endl;
  os << "ITRM: " << ITRM << endl;
  os << "ICNDIR: " << ICNDIR  << endl;
  os << "IGOTO: " << IGOTO << endl;
  os << "NAC: " << NAC << endl;
  os << "conminInfo: " << conminInfo  << endl;
  os << "INFOG: " << INFOG  << endl;
  os << "ITER: " << ITER  << endl;
  os << "numsamp: " << numsamp  << endl;
  os << "numNewPts: " << numNewPts << endl;
  os << "xMatrix" << endl; for(i=0;i<numsamp;i++) { os << xMatrix[i] << endl; }
  os << "yValueVector" << endl; for(i=0;i<numsamp;i++) { os <<  yValueVector[i] << endl; }
  os << "xNewVector" << endl; for(i=0;i<numsamp;i++) { os <<  xNewVector[i]  << endl; }
  os << "yNewVector" << endl; for(i=0;i<numsamp;i++) { os <<  yNewVector[i] << endl; }
  os << "betaHat: " <<  betaHat << endl; 
  os << "rhsTermsVector" << endl; for(i=0;i<numsamp;i++) { os <<  rhsTermsVector[i] << endl; }
  os << "iPivotVector" << endl; for(i=0;i<numsamp;i++) { os <<  iPivotVector[i] << endl; }
  os << "correlationMatrix" << endl; for(i=0;i<numsamp*numsamp;i++) { os <<  correlationMatrix[i] << endl; }
  os << "invcorrelMatrix" << endl; for(i=0;i<numsamp*numsamp;i++) { os <<  invcorrelMatrix[i] << endl; }
  os << "fValueVector" << endl; for(i=0;i<numsamp;i++) { os <<  fValueVector[i] << endl; }
  os << "fRinvVector" << endl; for(i=0;i<numsamp;i++) { os <<  fRinvVector[i] << endl; }
  os << "yfbVector" << endl; for(i=0;i<numsamp;i++) { os <<  yfbVector[i] << endl; }
  os << "yfbRinvVector" << endl; for(i=0;i<numsamp;i++) { os <<  yfbRinvVector[i] << endl; }
  os << "rXhatVector" << endl; for(i=0;i<numsamp;i++) { os <<  rXhatVector[i] << endl; }
  os << "workVector" << endl; for(i=0;i<numsamp;i++) { os <<  workVector[i] << endl; }
  os << "workVectorQuad" << endl; for(i=0;i<4*numsamp;i++) { os <<  workVectorQuad[i] << endl; }
  os << "iworkVector" << endl; for(i=0;i<numsamp;i++) { os <<  iworkVector[i] << endl; }
  os << "numSampQuad: " << numsamp  << endl;
  os << "conminSingleArray: " << numsamp  << endl;
}

//_____________________________________________________________________________
// Helper methods 
//_____________________________________________________________________________

void KrigingSurface::buildModel(SurfData& data)
{
  assert(xsize);
  int xsize_as_int = static_cast<int>(xsize);
  numsamp = static_cast<int>(data.size());
  for (unsigned i = 0; i < data.size(); i++) {
    for (unsigned j = 0; j < xsize; j++) {
       xMatrix[j*numsamp+i] = data[i].X()[j];
    }
    yValueVector[i] = data.getResponse(i);
  } 
  if (runConminFlag) {
    // call to F77 driver code that runs CONMIN and the kriging software
    conminInfo = 0; // this flag must be zero before calling CONMIN
    IGOTO      = 0; // initialize CONMIN's internal flag
    iFlag      = 1; // flag for kriging code: value=1 used w/ optimization

    // Print out xMatrix
    //cout << "xMatrix before call to conmin" << endl;
    //for (int i = 0; i < xsize*numsamp; i++) {
    //        cout << xMatrix[i] << endl;
    //}
    //cout << "yValueVector before call to conmin" << endl;
    //for (int j = 0; j < numsamp; j++) {
    //        cout << yValueVector[j] << endl;
    //}
    //writeMatrix("xMatrix before conmin",xMatrix,numsamp,xsize,cout);
    //writeMatrix("yVector before conmin",yValueVector,numsamp,1,cout);
#ifdef PRINT_DEBUG
    ofstream before("before.txt",ios::out);
    //ostream& os = cout;
    //os << "Before call to callconmin in buildModel" << endl;
    printConminVariables(before);
    before.close();
#endif
    CALLCONMIN_F77(conminThetaVars[0], conminThetaLowerBnds[0],
		   conminThetaUpperBnds[0],
		   constraintVector[0],SCAL[0],DF[0],A[0],S[0],G1[0],G2[0],
		   B[0],C[0],ISC[0],IC[0],MS1[0],N1,N2,N3,N4,N5,
		   DELFUN,DABFUN,FDCH,FDCHM,CT,CTMIN,CTL,CTLMIN,ALPHAX,ABOBJ1,THETA,
		   maxLikelihoodEst,xsize_as_int,numcon,NSIDE,IPRINT,NFDG,NSCAL,
		   LINOBJ,ITMAX,ITRM,ICNDIR,IGOTO,NAC,
		   conminInfo,INFOG,ITER,numsamp,numNewPts,iFlag, 
		   xMatrix[0],yValueVector[0],xNewVector[0],yNewVector[0],betaHat,
		   rhsTermsVector[0],iPivotVector[0],correlationMatrix[0],
		   invcorrelMatrix[0],fValueVector[0],fRinvVector[0], 
		   yfbVector[0],yfbRinvVector[0],rXhatVector[0],
		   workVector[0],workVectorQuad[0],iworkVector[0], 
		   numSampQuad,conminSingleArray);
#ifdef PRINT_DEBUG
    ofstream after("after.txt",ios::out);
    //os << "After call to callconmin in buildModel" << endl;
    printConminVariables(after);
    after.close();
#endif
    // copy CONMIN's theta variables to the thetaVector array for use in the
    // kriging model evaluation phase
    for (unsigned i=0;i<xsize;i++) {
      thetaVector[i] = conminThetaVars[i];
    }

    ////  run_conmin_flag = 0;
  }
  else {
    // Call to krigmodel() to bypass CONMIN. This approach uses the
    // user-supplied correlation values specified in the input file.
    iFlag = 1;
#ifdef PRINT_DEBUG
    ostream& os = cout;
    os << "Before call to krigmodel in buildModel" << endl;
    printKrigModelVariables(os);
#endif
    KRIGMODEL_F77(xsize_as_int,numsamp,numNewPts,
		  iFlag,thetaVector[0],xMatrix[0],yValueVector[0],xNewVector[0],
		  yNewVector[0],betaHat,rhsTermsVector[0],maxLikelihoodEst,
		  iPivotVector[0],correlationMatrix[0],invcorrelMatrix[0],
		  fValueVector[0],fRinvVector[0],yfbVector[0],yfbRinvVector[0],
		  rXhatVector[0],workVector[0],workVectorQuad[0],iworkVector[0],
		  numSampQuad);


#ifdef PRINT_DEBUG
    os << "After call to krigmodel in buildModel" << endl;
    printKrigModelVariables(os);
#endif
  }
}

//_____________________________________________________________________________
// Testing 
//_____________________________________________________________________________

#ifdef __TESTING_MODE__
  int KrigingSurface::constructCount = 0;
  int KrigingSurface::copyCount = 0;
  int KrigingSurface::destructCount = 0;
#endif

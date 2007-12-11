#include "surfpack_system_headers.h"
#include "RadialBasisFunctionModel.h"
#include "surfpack.h"
#include "AxesBounds.h"
#include "ModelFitness.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;

SurfPoint computeCentroid(const SurfData& sd)
{
  assert(sd.size());
  assert(sd.xSize());
  VecDbl center(sd.xSize(),0.0);
  for (unsigned pt = 0; pt < sd.size(); pt++) {
    for (unsigned dim = 0; dim < sd.xSize(); dim++) {
      center[dim] += sd(pt,dim);
    }
  }
  for (unsigned dim = 0; dim < center.size(); dim++) {
    center[dim] /= sd.size();
  }
  return SurfPoint(center);
}

void updateCentroid(VecDbl& centroid, const VecDbl& newpt, unsigned weight)
{
  assert(centroid.size() == newpt.size());
  for (unsigned i = 0; i < centroid.size(); i++) {
    if (!weight) centroid[i] = newpt[i];
    else centroid[i] = (weight*centroid[i]+newpt[i])/(weight+1);
  }
}

unsigned findClosest(const SurfData& sd, VecDbl pt)
{
  assert(sd.size());
  double mindist = surfpack::euclideanDistance(sd(0),pt);
  unsigned argmin = 0;
  for (unsigned i = 1; i < sd.size(); i++) {
    double distance = surfpack::euclideanDistance(sd(i),pt);
    if (distance < mindist) {
      mindist = distance;
      argmin = i;
    }
  }
  return argmin;
}

SurfData radii(const SurfData& generators)
{
  SurfData result;
  for (unsigned i = 0; i < generators.size(); i++) {
    VecDbl radius(generators.xSize(),std::numeric_limits<double>::max());
    for (unsigned j = 0; j < generators.size(); j++) {
      if (i != j) {
        for (unsigned dim = 0; dim < generators.xSize(); dim++) {
          double distance = fabs(generators(i,dim)-generators(j,dim));
          if (distance < radius[dim]) radius[dim] = distance;
        }
      }
    }
    result.addPoint(SurfPoint(radius));
  }
  return result;
}

SurfData cvts(const AxesBounds& ab)
{
  unsigned q = 100; // number of sample points for Ju-Du-Gunzburger alg.
  unsigned g = 50; // number of sample points for Ju-Du-Gunzburger alg.
  double minalpha = .5;
  double maxalpha = .99;
  SurfData* generators = ab.sampleMonteCarlo(g);
  unsigned iters = 10;
  for (unsigned i = 0; i < iters; i++) {
    SurfData* samples = ab.sampleMonteCarlo(q);
    vector<SurfData> closestSets(g);
    for (unsigned samp = 0; samp < samples->size(); samp++) {
      closestSets[findClosest(*generators,(*samples)(samp))].addPoint((*samples)[samp]);
    } // for each sample pt
    // Find centroids, update generators
    SurfData* new_generators = new SurfData;
    for (unsigned gen = 0; gen < g; gen++) {
      if (closestSets[gen].size() != 0) {
        SurfPoint center = computeCentroid(closestSets[gen]);
        double genweight = minalpha + (maxalpha - minalpha)*((double)i/iters);
        new_generators->addPoint(SurfPoint(
          surfpack::weightedAvg((*generators)(gen),center.X(),genweight)));
      } else {
        new_generators->addPoint((*generators)(gen));
      }
    }
    delete generators; generators = new_generators;
    delete samples;
  } // end iteration
  SurfData result(*generators);
  delete generators;
  return result;
}

VecRbf makeRbfs(const SurfData& generators, const SurfData& radii)
{
  assert(generators.size());
  assert(generators.size() == radii.size());
  vector<RadialBasisFunction> rbfs;
  for (unsigned i = 0; i < generators.size(); i++) {
    rbfs.push_back(RadialBasisFunction(generators(i),radii(i)));
  }
  return rbfs;
}

// Add additional rbfs with broader support to set of candidates
void augment(VecRbf& rbfs)
{
  assert(rbfs.size());
  unsigned toAdd = rbfs.size(); 
  for (unsigned i = 0; i < toAdd; i++) {
    unsigned first = rand() % rbfs.size();
    unsigned second = rand() % rbfs.size();
    //cout << "new basis from " << first << " " << second << endl;
    VecDbl newRadius = rbfs[first].radius;
    if (first == second) { // new function with same center/double radius
      for (unsigned dim = 0; dim < newRadius.size(); dim++) {
        newRadius[dim] *= 2.0;
      }
      rbfs.push_back(RadialBasisFunction(rbfs[first].center,newRadius));
    } else { // new function with avg center, sum of radii
      VecDbl newCenter = surfpack::weightedAvg(rbfs[first].center,rbfs[second].center);
      for (unsigned dim = 0; dim < newRadius.size(); dim++) {
        newRadius[dim] += rbfs[second].radius[dim];
      }
      rbfs.push_back(RadialBasisFunction(newCenter,newRadius));
    }
  }
}

MtxDbl getMatrix(const SurfData& sd, const VecRbf& candidates, const VecUns& used)
{
  assert(candidates.size() >= used.size());
  MtxDbl A(sd.size(),used.size(),true);
  for (unsigned rowa = 0; rowa < A.getNRows(); rowa++) {
    for (unsigned cola = 0; cola < A.getNCols(); cola++) {
      A(rowa,cola) = candidates[used[cola]](sd(rowa));
    }
  }
  return A;
}

VecUns probInclusion(unsigned vec_size, double prob)
{
  assert(prob >= 0.0);
  assert(prob <= 1.0);
  assert(vec_size);
  VecUns result;
  for (unsigned i = 0; i < vec_size; i++) {
    if ((double)rand()/INT_MAX < prob) result.push_back(i);
  }
  return result;
}

VecDbl fullCoeff(unsigned vec_size, const VecDbl& coeffs, VecUns& incl)
{
  VecDbl result(vec_size,0.0);
  for (unsigned i = 0; i < incl.size(); i++) {
    result[incl[i]] = coeffs[i];
  }
  return result;
}

///\todo The exp() function can eat up a lot of time in this method
/// If it becomes a bottleneck, switch to a cached lookup table for
/// the values of exp(x)
//const unsigned granularity = 1000;
//const double maxe = 8.0;
//VecDbl initExps()
//{
//  VecDbl exps;
//  exps.reserve(granularity);
//  for (unsigned i = 0; i < granularity; i++) {
//    exps.push_back(exp(-(double)i/granularity*maxe));
//  }
//  cout << "Initialized " << endl;
//  return exps;
//}
//
//double myexp(const double x)
//{
//  assert(x >= 0.0);
//  static VecDbl exps(initExps());
//  if (x > maxe) return 0.0;
//  return exps[(unsigned)(x/maxe*granularity)];
//}

RadialBasisFunction::RadialBasisFunction(const VecDbl& center_in, const VecDbl& radius_in)
  : center(center_in), radius(radius_in)
{
  assert(!center.empty());
  assert(center.size() == radius.size()); 
}

RadialBasisFunction::RadialBasisFunction(const std::string& center_in, const std::string& radius_in)
  : center(surfpack::toVec<double>(center_in)),
  radius(surfpack::toVec<double>(radius_in))
{
  assert(!center.empty());
  assert(!radius.empty());
  assert(center.size() == radius.size()); 
}

double RadialBasisFunction::operator()(const VecDbl& x) const
{
  assert(x.size() == center.size());
  double sum = 0.0;
  double temp;
  for (unsigned i = 0; i < center.size(); i++) {
    temp = x[i] - center[i];
    sum += temp*temp*radius[i];
  };
  //return myexp(sum);
  return exp(-sum);
}

double RadialBasisFunction::deriv(const VecDbl& x, const VecUns& vars) const
{
  assert(vars.size() == 1);
  assert(!center.empty());
  assert(!radius.empty());
  assert(x.size() == center.size());
  unsigned i = vars[0];
  return -2.0*radius[i]*(x[i]-center[i])*(*this)(x);
}

std::string RadialBasisFunction::asString() const
{
  std::ostringstream os;
  os << "center: ";
  copy(center.begin(),center.end(),std::ostream_iterator<double>(os," "));
  os << " radius: ";
  copy(radius.begin(),radius.end(),std::ostream_iterator<double>(os," "));
  return os.str();
}

RadialBasisFunctionModel::RadialBasisFunctionModel(const VecRbf& rbfs_in, const VecDbl& coeffs_in)
  : SurfpackModel(1), rbfs(rbfs_in),coeffs(coeffs_in)
{
  assert(!rbfs.empty());
  this->ndims = rbfs[0].center.size();
  assert(this->size() != 0);
  assert(rbfs.size() == coeffs.size()); 
}

double RadialBasisFunctionModel::evaluate(const VecDbl& x) const
{
  double sum = 0.0;
  for (unsigned i = 0; i < rbfs.size(); i++) {
    sum += coeffs[i]*rbfs[i](x);
  }
  return sum;
}

/// Currently set up so that operator() must be called immediately before
/// Not good assumption
VecDbl RadialBasisFunctionModel::gradient(const VecDbl& x) const
{
  /// code copied straight from LRM
  assert(!x.empty());
  //assert(coeffs.size() == bs.bases.size());
  VecUns diff_var(1,0); // variable with which to differentiate
  VecDbl result(x.size(),0.0);
  for (unsigned i = 0; i < x.size(); i++) {
    diff_var[0] = i;
    for (unsigned j = 0; j < rbfs.size(); j++) {
      result[i] += coeffs[j]*rbfs[j].deriv(x,diff_var);
    }
  }
  return result;
}

std::string RadialBasisFunctionModel::asString() const
{
  std::ostringstream os;
  for (unsigned i = 0; i < rbfs.size(); i++) {
    os << coeffs[i] << " * " << rbfs[i].asString() << "\n";
  }
  return os.str();
}

typedef std::pair<double,VecUns> RbfBest;
///////////////////////////////////////////////////////////
///	Moving Least Squares Model Factory
///////////////////////////////////////////////////////////

RadialBasisFunctionModelFactory::RadialBasisFunctionModelFactory()
  : SurfpackModelFactory(), nCenters(0), minPartition(1)
{

}

RadialBasisFunctionModelFactory::RadialBasisFunctionModelFactory(const ParamMap& args)
  : SurfpackModelFactory(args), nCenters(0), minPartition(1)
{

}

void RadialBasisFunctionModelFactory::config()
{
  SurfpackModelFactory::config();
  string strarg;
  strarg = params["centers"];
  if (strarg != "") nCenters = atoi(strarg.c_str());
  strarg = params["min_partition"];
  if (strarg != "") minPartition = atoi(strarg.c_str());
  //strarg = params["cvt_pts"];
  //if (strarg != "") cvtPts = atoi(strarg.c_str());
  //strarg = params["trials"];
  //if (strarg != "") trials = atoi(strarg.c_str());
}

SurfpackModel* RadialBasisFunctionModelFactory::Create(const SurfData& sd)
{
  if (nCenters == 0) nCenters = sd.size();
  RbfBest bestset(std::numeric_limits<double>::max(),VecUns());
  SurfData centers = cvts(AxesBounds::boundingBox(sd));
  printf("data points: %d\n",sd.size());
  printf("centers: %d\n",centers.size());
  SurfData radiuses = radii(centers);
  printf("radii: %d\n",radiuses.size());
  VecDbl b = sd.getResponses();
  VecRbf candidates = makeRbfs(centers,radiuses);
  printf("candidates: %d\n",candidates.size());
  augment(candidates);
  VecDbl cfs(candidates.size(),0.0);
  for (unsigned i = 0; i < 50; i++) {
    VecUns used = probInclusion(candidates.size(),.5);
    MtxDbl A = getMatrix(sd,candidates,used);
    VecDbl x;
    surfpack::linearSystemLeastSquares(A,x,b);
    VecDbl coeffs = fullCoeff(candidates.size(),x,used);
    RadialBasisFunctionModel rbfm(candidates,coeffs);
    StandardFitness sf;
    double fitness = sf(rbfm,sd);
    //cout << "#rbfs: " << used.size() << " fitness: " << fitness << endl;
    if (fitness < bestset.first) bestset = RbfBest(fitness,used);
  }
  //cout << "Best fitness: " << bestset.first << endl;
  // Now create an RBF set that only includes the ones that have
  // non-zero coefficients
  VecUns used = bestset.second;
  VecRbf final_rbfs;
  VecUns final_used(used.size());
  for (unsigned i = 0; i < used.size(); i++) {
    final_used[i] = i;
    final_rbfs.push_back(candidates[i]);
  }
  // Recompute the coefficients.  If we cached the result, we wouldn't
  // have to do it again.  
  MtxDbl A = getMatrix(sd,final_rbfs,final_used);
  VecDbl x;
  surfpack::linearSystemLeastSquares(A,x,b);
  SurfpackModel* sm = new RadialBasisFunctionModel(final_rbfs, x); 
  assert(sm);
  return sm; 
}

SurfpackModel* RadialBasisFunctionModelFactory::Create(const std::string& model_string)
{
  ///\todo Be able to parse an RBF model from a string
  assert(false);
  return 0;
}


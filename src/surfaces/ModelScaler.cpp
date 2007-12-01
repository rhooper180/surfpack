#include "surfpack_system_headers.h"
#include "ModelScaler.h"
#include "SurfData.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;

const VecDbl& NonScaler::scale(const VecDbl& unscaled_x) const 
{ 
  return unscaled_x;
}

double NonScaler::descale(double scaled_response) const 
{
  return scaled_response;
}

double NonScaler::scaleResponse(double unscaled_response) const 
{
  return unscaled_response; 
}

std::string NonScaler::asString()
{
  return string("No scaling");
}

ModelScaler* NonScaler::clone() const
{
  return new NonScaler(*this);
}

const VecDbl& NormalizingScaler::scale(const VecDbl& unscaled_x) const
{
  //cout << "NormalizingScaler::scale" << endl;
  assert(unscaled_x.size() == scalers.size());
  assert(this->result.size() == scalers.size());
  for (unsigned i = 0; i < scalers.size(); i++) {
    this->result[i] = (unscaled_x[i] - scalers[i].offset)/scalers[i].scaleFactor;
  }
  return this->result;
}

double NormalizingScaler::descale(double scaled_response) const
{
  return scaled_response*descaler.scaleFactor + descaler.offset;
}

double NormalizingScaler::scaleResponse(double unscaled_response) const 
{
  return (unscaled_response - descaler.offset) / descaler.scaleFactor;
}

NormalizingScaler* NormalizingScaler::Create(const SurfData& data)
{
  vector<NormalizingScaler::Scaler> scalers(data.xSize());
  for (unsigned i = 0; i < data.xSize(); i++) {
    VecDbl predictor = data.getPredictor(i);
    scalers[i].offset = *(std::min_element(predictor.begin(),predictor.end()));
    scalers[i].scaleFactor = 
      *(std::max_element(predictor.begin(),predictor.end())) - scalers[i].offset;
  }
  NormalizingScaler::Scaler descaler;
  VecDbl response = data.getResponses();
  descaler.offset = *(std::min_element(response.begin(),response.end()));
  descaler.scaleFactor = 
    *(std::max_element(response.begin(),response.end())) - descaler.offset;
  return new NormalizingScaler(scalers,descaler);
}

ModelScaler* NormalizingScaler::clone() const
{
  return new NormalizingScaler(*this);
}

std::string NormalizingScaler::asString()
{
  std::ostringstream os;
  for (unsigned i = 0; i < scalers.size(); i++) {
    os << "offset: " << scalers[i].offset 
       << " scaleFactor: " << scalers[i].scaleFactor
       << "\n";
  }
  os << "descaler offset: " << descaler.offset << " scaleFactor: " << descaler.scaleFactor << endl;
  return os.str();
}

/// ScaledSurfData
ScaledSurfData::ScaledSurfData(const ModelScaler& ms_in, const SurfData& sd_in)
  : ms(ms_in), sd(sd_in)
{

}

VecDbl ScaledSurfData::getResponses() const
{
  VecDbl responses = sd.getResponses();
  for (VecDbl::iterator it = responses.begin(); it != responses.end(); ++it) {
    *it = ms.scaleResponse(*it);
  }
  return responses;
}

double ScaledSurfData::getResponse(unsigned index) const
{
  //checkRangeNumPoints(header, index);
  double unscaled = sd.getResponse(index);
  return ms.scaleResponse(unscaled);
}

unsigned ScaledSurfData::size() const
{
  return sd.size();
}

unsigned ScaledSurfData::xSize() const
{
  return sd.xSize();
}

double ScaledSurfData::operator()(unsigned pt, unsigned dim) const
{
  assert(pt < sd.size());
  assert(dim < sd.xSize());
  //return points[mapping[pt]]->X()[dim];
  const VecDbl& unscaled_pt = sd[pt].X();
  const VecDbl& scaled_pt = ms.scale(unscaled_pt);
  return scaled_pt[dim];
}

const VecDbl& ScaledSurfData::operator()(unsigned pt) const
{
  assert(pt < sd.size());
  const VecDbl& unscaled_pt = sd[pt].X();
  return ms.scale(unscaled_pt);
  //copy(scaled_pt.begin(),scaled_pt.end(),std::ostream_iterator<double>(cout," "));
}

VecVecDbl ScaledSurfData::asVecVecDbl(const ScaledSurfData& data)
{
  VecVecDbl result(data.size(),data.xSize());
  for (unsigned i = 0; i < data.size(); i++) {
    for (unsigned j = 0; j < data.xSize(); j++) {
      result[i][j] = data(i,j);
    }
  }
  return result;
}
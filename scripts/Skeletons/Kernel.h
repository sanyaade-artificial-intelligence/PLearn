#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include "Kernel.h"

namespace PLearn {
using namespace std;

class DERIVEDCLASS: public Kernel
{

private:

  typedef Kernel inherited;
  
protected:

  // *********************
  // * Protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...
    
public:

  // ************************
  // * Public build options *
  // ************************

  // ### declare public option fields (such as build options) here
  // ...

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  // Make sure the implementation in the .cc initializes all fields to
  // reasonable default values.
  DERIVEDCLASS();

  // ******************
  // * Kernel methods *
  // ******************

private: 

  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  
  //! Declares this class' options.
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declares other standard object methods.
  // If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS.
  PLEARN_DECLARE_OBJECT(DERIVEDCLASS);

  // **************************
  // **** Kernel methods ****
  // **************************

  //! Compute K(x1,x2).
  virtual real evaluate(const Vec& x1, const Vec& x2) const;

  // *** SUBCLASS WRITING: ***
  // While in general not necessary, in case of particular needs 
  // (efficiency concerns for ex) you may also want to overload
  // some of the following methods:
  // virtual real evaluate_i_j(int i, int j) const;
  // virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const;
  // virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const;
  // virtual real evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x=-1, bool first_time = false) const;
  // virtual real evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x=-1, bool first_time = false) const;
  // virtual void computeGramMatrix(Mat K) const;
  // virtual void setDataForKernelMatrix(VMat the_data);
  // virtual void addDataForKernelMatrix(const Vec& newRow);
  // virtual void setParameters(Vec paramvec);
  // virtual Vec getParameters() const;
  

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(DERIVEDCLASS);
  
} // end of namespace PLearn

#endif


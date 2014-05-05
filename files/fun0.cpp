#ifndef %1
#define %1
#include "common.h"

#include "rebound_PB.h"
//! Roadblock Function Template
void /*Name*/(/*Arguments*/,LINCS &lincs)
{
    const double sDT=std::sqrt(lincs.beadDiffusion()*lincs.timeStep);
    const double s5DT=std::sqrt(5.0)*sDT;
    for(int i=0; i<lincs.R.size(); ++i)
    {
        const Vector3d &r = lincs.R[i];
        //! @/*name*/{ f(r) = /*expresion*/; df(r) = /*gradient*/; /*the_dimension_of_f*/}
        //! #Assemble = /*assemble*/
    }
}

//! Force Function Template
void /*Name*/(/*Arguments*/,LINCS &lincs)
{
    //lincs.plusF(/*beadId*/,/*Vector3d*/);
    //lincs.minusF(/*beadId*/,/*Vector3d*/);
}

//! Constraint Function Template
unsigned /*Name*/(/*beadId*/,LINCS &lincs)
{
    /*...*/
    lincs.setGCerror(/*error*/);
    lincs.setGCdirection(/*beadId*/,/*Vector3d*/);
    return lincs.endInsertOneGC();
}

#endif


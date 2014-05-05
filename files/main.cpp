#include <fstream>
#include <ctime>
#include <assert.h>
#include "common.h"

int main()
{
    //! [Parameter]
    const double temperature = 298;
    const double viscosity = 0.0008904;
    const double timeStep = ?;
    const unsigned long steps=?;
    //! [Parameter end]

    //! [Output]
    unsigned long count = 0;

    //! [Output end]
    LINCS lincs(temperature,viscosity,timeStep);
    lincs.setSeed(BD_SEED);
    for(unsigned long istep=0;istep<steps;++istep)
    {
        //! [Output lincs]
        if(/*output_condition*/)
        {

        }
        //! [Output lincs end]
    }
    //! [Output out]

    //! [Output out end]
}


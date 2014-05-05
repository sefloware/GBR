//! title one 0
//! [%1 chain]
VectorXV3d %1(/*bead_count*/);
//initialize ...
//! [%1 chain end]
//! title one 0 end

//! title one 1
//! [%1 chain lincs]
lincs.swapBeadPositions(%1);
lincs.resizeGC(/*constraint_count*/);
lincs.setBeadRadius(/*bead_radius*/);
lincs.reset();
//forces,constraints,...

lincs();
lincs.swapBeadPositions(%1);
//! [%1 chain lincs end]
//! title one 1 end

//! title array 0
//! [%1 chain]
VectorXV3d %1;
for(int i=0;i<%3;++i)
    %2[i].resize(/*bead_count*/);
//initialize ...
//! [%1 chain end]
//! title array 0 end

//! title array 1
//! [%1 chain lincs]
for(int i=0;i<%3;++i)
{
    lincs.swapBeadPositions(%2[i]);
    lincs.resizeGC(/*constraint_count*/);
    lincs.setBeadRadius(/*bead_radius*/);
    lincs.reset();
    //forces,constraints,...

    lincs();
    lincs.swapBeadPositions(%2[i]);
}
//! [%1 chain lincs end]
//! title array 1 end


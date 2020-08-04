//Eigen Stuff
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>
#include <Eigen/SparseCholesky>
#include <Eigen/IterativeLinearSolvers>
#include <glm/glm.hpp>

#include <iostream>

// Core Stuff
#include "src/algebra.h"    //Space Discretizations
#include "src/shape.h"      //Shape Functions

#include "src/space.h"      //Space Discretizations
#include "src/time.h"       //Time Discretizations and Integrators
#include "src/solve.h"      //Full Scheme Solvers / Algorithms
#include "src/source.h"     //Physical Source and Sink Terms

#include "helpers/timer.h"    //Benchmarking Tool

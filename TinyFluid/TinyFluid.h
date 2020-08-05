/*
TinyFluid - Small Differential Equation Solver
Author: Nicholas McDonald
*/

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>
#include <glm/glm.hpp>

#include <iostream>

// Core Stuff
#include <math.h>

#include "include/src/algebra.h"    //Space Discretizations
#include "include/src/shape.h"      //Shape Functions

#include "include/src/space.h"      //Space Discretizations
#include "include/src/time.h"       //Time Discretizations and Integrators
#include "include/src/solve.h"      //Full Scheme Solvers / Algorithms

#include "include/helpers/timer.h"    //Benchmarking Tool

//#include "render/view.cpp"
//#include "render/input.cpp"

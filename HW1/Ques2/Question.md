Implement the function analysis code in a parallel scheme: 

Download the code for this problem: p2_serial.c and p2_func.c

This code implements the serial version of the numerical techniques described below. You will update this code to run in parallel using MPI.
Given some function, the code will numerically compute derivatives at all grid points, integral of the function over the grid points, and the error and standard deviation of the error of the numerical computation.

IMPORTANT: You MUST supply a function that is continuous and smooth - that is, that has an analytic first derivative. Examples include:

 x^2
 sqrt(x)
 sin(2 * x)
For a list of functions, their derivatives and integrals, see here.
IMPORTANT: Be sure to choose a function that doesn't behave weird between the user-defined grid values XI and XF. That is, do not do illegal math operations like fn(x) = 1/x on the range with a grid value x = 0.0, or fn(x) = sqrt(x) with a grid value x = -1.0.

Write a Makefile to compile the code with:

gcc -g -o p2 p2.c p2_func.c -lm
The program will output a file err.dat which gives:

The average error in the derivative calculation;
The standard deviation of the error in the derivative calculation;
The relative error of the integral calculation;
A point-by-point list of the error at each grid point.
Edit the program to include more grid points, different functions, etc.
Data Decomposition

A common approach to implementing a parallel program is data decomposition. The data the program will run with is split up into chunks and a copy of the program runs using each individual chunk.

Consider the array of N grid points x_1 to x_n:

For our program, we will split this grid into evenly spaced "slices" by processor rank:

IMPORTANT: Be sure that the number of grid points is evenly divisible by the number of processors, or make sure your final code handles the case where the number of processors does not evenly divide the number of grid zones.

Computing derivatives: Finite Differencing

It is often the case that we cannot analytically calculate the derivative of some data. However, we can approximate it using a finite differencing scheme.

The method comes from calculus. Recall the definition of a derivative of the function f(x):

This is a good approximation, but a better one is to use the symmetric form of the above:

In our program, let xc[i] be our grid points, and let yc[i] be the value of our function at grid point xc[i]. Using the above, we can approximate the derivative at the grid point xc[i] as:

dydx[i] = (yc[i+1] - yc[i-1]) / (2.0 * dx)
We should notice two things: (i) We need to know the boundary conditions of our full grid domain - that is, what the value of the function will be at the left and right edge of the grid, and (ii) For our decomposed grid, we will need to communicate the boundary values of our function to the processors to the "left" and "right". The diagram below represents one side of this process. The process of rank i will receive the value fn(x_4) (fn(x) represents the value of our function at point x) from the process of rank i+1 send the value of the function fn(x_3) to the process of rank i+1

Computing integrals: Trapezoidal Rule

Similar to derivatives, integrals of data values must usually be carried out numerically. Here we will use a simple method to approximate our integral using the trapezoidal rule

That is, we can compute one integral for each pair of grid points x_i and x_i+1, then add them together at the end.

To compute each integral, recall that the integral of a function can be described as the area under the curve. Like with the finite difference, if the distance between two grid points is small, we can make a good approximation the integral between two points by assuming the function is a straight line between the two points. Then, we just have to compute the area of a trapezoid:

Actually, we are computing the area of a rectangle where the center of the top edge coincides with the center of the interval we are integrating over, and the height of the rectangle is the average value of the function evaluated at the endpoints. In fact, this is equivalent to a trapezoid, but calculating the area of a rectangle is easier. Graphically, the procedure looks like this:

For our parallel implementation, we again need to know the values across domain boundaries. Notice that we have already exchanged this information when calculating the derivatives, so we can simply use it again here. Also notice that, if there are N grid points on the domain, we calculate N - 1 integrals.

Error analysis: Mean and standard deviation

By now we know how to calculate good approximations of derivatives and integrals. However, we should recall that these are approximations, and the approximations we've been using are only good, not great. Also take note that these approximations only work with a specific class of functions that fall within narrow parameters. You can think about what happens to the finite difference method if the function is defined piece-wise (instead of continuously), or a function that varies rapidly (e.g. f(x) = sin(1000 * x))

Like with most numerical methods, determining error is important. There are many procedures to do so, and we will look at a simple but powerful error indicator: standard deviation

Standard deviation is simply a measure of how much all data points are spread out from their "expected" values. A small standard deviation means the data is "good", while a large standard deviation means the data is, maybe not necessarily poor, but inconsistent.

We will look at the standard deviation of the relative error for the derivatives:



Note we will need the actual values of the derivatives here, so our function must have an exact expression for the derivative.
IMPORTANT: Be careful not to divide by zero here; choose a function whose derivative does not evaluate to zero on any of the grid points.

To find standard deviation, we must first calculate the mean (or average) error of the entire domain. So first we calculate the error at each grid point.

The standard deviation is then given as:

Problem procedure

Using the provided serial code, the assignment is as follows:
Decompose the grid into slices based on processor rank.
Calculate the derivative of the functions using a finite difference method. To properly calculate the derivative at the domain boundaries, use (a) a blocking point-to-point communication and (b) a non-blocking point-to-point communication.
Calculate the integral of the function using the trapezoidal rule. Here, we will require again a communication across the boundary, as well as a global reduction sum.
Calculate the errors in the derivatives and in the integral. Communicate a vector of errors for each grid point to the root node, and have the root node calculate the average error and the standard deviation the derivatives, and the relative error in the calculation of the integral.
Some guidelines:

Your program should produce accurate results; be sure to compare the output of the serial code to the parallel code.
Your program should be robust enough to do all operations with a wide range of input parameters (e.g. number of grid points, number of processors used).
Run your program with several different functions; your code will be tested with a unique function to check it runs correctly.
Compare the performance (using MPI_Wtime) for long-running inputs (large number of grid points) for each calculation (finite difference, integral, error), and each communication method (blocking/non-blocking point-to-point, single call/manual reduction) with submitted jobs (to ensure low contention). Show your results and comment on the outcome in the README file. Comment on how the errors change for (i) the number of grid points and (ii) the function used. The sample output of err.dat is here.
NOTE:The above output was created using the function fn(x) = sqrt(x)
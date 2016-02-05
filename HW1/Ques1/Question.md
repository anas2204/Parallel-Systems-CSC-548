Write an MPI program that determines the point-to-point message latency for pairs of nodes. Exchange point-to-point message with varying message sizes (32B, 64B, 128B, ... , 2M). Your program should run continuously (i.e., do not re-run for each message size - your program should iterate through all different message sizes and repeat each message size 10 times). You need to run on (a) 8 nodes and (b) 16 nodes.
Plot the average time for point-to-point message round-trip time (rtt) as a function of message size for each node pair for (a) and (b), as well as the standard deviation (stddev) as error bars.

In a README file, explain your plots. Do some nodes take longer than others? In particular, could your results be indicative of the underlying network configuration? Explain. Also discuss message size as it relates to latency; are there any odd data points in your graphs?

Hints:

To ensure only one process per node, use this job submit parameters in your PBS script:
#PBS -l nodes=<# of nodes>:ppn=1
When calculating average rtt, skip the first message exchange -- why?
Ensure that only two nodes are exchanging messages at any one time -- why?
Use your favorite plotting program; we recommend gnuplot, but any program that graphs data is fine.
It may be very useful to use logscale for both x,y axis of your plots.
Sample output (of not necessarily realistic numbers): read as follows [message size] [node_1_avg node_1_stddev] [node_2_avg node_2_stddev] ...

8 8.276531e-06 5.960464e-08 5.517687e-06 3.814697e-09 ...

Turn in the files p1.c, p1.Makefile, rrt8.png, rtt16.png, and p1.README
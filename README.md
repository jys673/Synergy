Synergy Sample Applications
===========================

Stateless Parallel Processing using Tuple Switching Network

Stateless parallel processing is based on the assumption that remote execution and tranmission state is not reliable.
Therefore, it is necessary to introduce a layer of tuple-switching network in order to shield the applications from
the direct harms of data losses.

This repo contains three types of Synergy applications:

a) SSC (Solution Space Compact) applications. For these applications, it is NOT possible to generate superlinear speedups by manipulating input data. 

b) NSSC (Non-Solution Space Compact) applications. For these applications, it is possible to generate superlinear speedups by manipulating input data. All NP-complete algorithms are candidate applications. The condition to qualify an application to this category is to contrast the applications solution algorithm's complexity against the solution certificate (result validation) complexity. If the result is at least one-order of magnitude, then the application is NSSC.

c) PML applications. PML stands for Parallel Markup Language. It is used to markup sequential programs in order to generate data parallel programs automatically. This project is still very young. Documentation is primarily the Dissertation by Feijian Sun at Temple University.

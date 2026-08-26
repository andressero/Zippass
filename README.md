# Zippass

Unlock zip passwords.

This is a project for the course "Programación Paralela y Concurrente", taken in the college "Universidad de Costa Rica".

Given a set of unique characters (an alphabet), this application will brute force zip archives with password protection. 

The project shows different approaches to solving the problem:

- **Zippass Serial**, as its name implies, is a single threaded program. Although the program is memory efficient, the brute forcing takes a long time.
- **Zippass Thread** introduces the usage of Pthreads to parallelize the workload and reduce the time needed to find the password. The number of threads defaults to the number of CPU cores the computer has, but any other number can be specified.
- **Zippass OMP MPI** solves the problem using threads again, but with OpenMP instead of Pthreads, which simplifies the code. It also introduces MPI, a technology to communicate processes, meaning that the workload can now be distributed among a network of PCs, in a cluster for example.
- **Zippass Optimized**, is the same as Zippass OMP MPI but implements a few optimizations.


# How to run

1. Install dependencies

For Ubuntu/Debian:

```shell
sudo apt install make gcc build-essential libzip-dev
```
2. Clone this repo

```shell
git clone https://github.com/andressero/Zippass.git
```

3. Navigate to one of the projects

For example, to navigate to Zippass Optimized:
```shell
cd zippass_optimized
```

4. Compile the project
5. Execute the program
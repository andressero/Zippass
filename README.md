# Zippass

A tool to brute force password protected zip archives.

This is a project for the course "Programación Paralela y Concurrente", taken in the college "Universidad de Costa Rica".

Given a set of unique characters (an alphabet) and a maximum length, this application will attempt to recover passwords from protected ZIP archives. 

To solve the problem, several approaches were explored:

- **Zippass Serial**, is a single-threaded program. For that reason, the brute force process takes a long time.
- **Zippass Thread** introduces the usage of Pthreads to parallelize the workload and reduce execution time. The number of threads defaults to the number of CPU cores available, but a different number can be specified.
- **Zippass OMP MPI** uses threads from OpenMP instead of Pthreads, which simplifies the code. It also introduces MPI, a technology to communicate processes, meaning that the workload can now be distributed among a network of PCs, in a cluster for example.
- **Zippass Optimized**, takes Zippass Thread as a base and implements a few optimizations.


# Preparations

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

4. Use the provided Makefile to build an optimized executable
```shell
make release
```

# Usage

The program expects a specific structure from stdin, ideally from a text file. You can create a text file that looks like this:

```txt
abcdefghijklmnopqrstuvwxyz
5

home/user/Documents/zips/important_data.zip
home/user/Documents/zips/Classified.zip
home/user/Documents/zips/Critical_Backups.zip
```

Where:

- The first line contains the alphabet.
- The second line specifies the maximum length of the password.
- The third line must be blank
- Starting from the fourth line, each line contains the path to an encrypted zip archive.

Once the file is ready, you can run the program and provide the text file through stdin like this:
```shell
bin/zippass_optimized < textfile.txt
```

To run the program with a different amount of threads, provide it as an argument:
```shell
bin/zippass_optimized 16 < textfile.txt
```

You can also redirect the program's output to a file like this:
```shell
bin/zippass_optimized < textfile.txt > brute_forced_passwords.txt
```

# How it works



# What is being parallelized


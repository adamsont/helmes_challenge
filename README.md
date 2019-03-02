# Helmes challenge
Anagram finder for https://www.helmes.com/careers/challenge/

# Building

Build app with command:
```
gcc -o app main.c -O3
```
Optimization flag -O3 is vital to performance.

# Running

Provided executable is built for: https://hub.docker.com/_/gcc

Example:
```
./app lemmad.txt ilves
```

# Input file
App expects windows-style line endings and 
no other characters than ones provided in challenge input file:
http://www.eki.ee/tarkvara/wordlist/lemmad.zip

Input file can be subset of provided file as long as previous expectations remain true.

Anagrams are found case-insensitive

# Output
App outputs run time in us (microseconds)
And all found anagrams as comma-separated list

# Additinal files
Some additional optional convenience files are provided:
* Dockerfile - to build example docker container
* build.bat - to build app and container
* run.bat - to run app in docker container
* lemmad.txt - original test file

Windows users:
```
docker pull gcc
build
run lemmad.txt ilves
```

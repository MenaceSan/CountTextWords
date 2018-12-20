# NetApp SSFI Interview application
AKA CountTextWords
By Dennis Robinson (dennis@menasoft.com)
2018-12-01
A NetApp interview C++ problem based on "Element Core Dev Coding Assignment.docx"

### Summary
Create an app that reads all '*.txt' files in a given directory.
Create one directory enumeration thread and N other threads to read the actual file contents.
The reader threads should count the number of occurrences of each word. Words are delimited by non alphanumerics.
When complete, print out a list of the top ten occurring words.

### Goals:

Fastest possible run time. Optimized for large number of files (A) preferred over other optimizations.
Avoid slower interfaces. e.g. using shell for directory list.
Keep it small and simple. Write Less code. Easy to read.
Dev Time limited. Can only spend so much time on this.
Portable to Windows and most Linux distros.
-Help mode.
Show run timing.
Fail gracefully.
Commented code.
Use STL for all possible.
Clean cancellation in windows.
No build warnings
use const keyword appropriately for best optimization.

### Assumptions:

c++17 for std::filesystem support on MSVC. experimental for g++.

Most files will be serialized to a single device so threading might not be effective. 
Most operations will be i/O bound. SAN arrays may break that rule.
Number of cores on the machine = optimal number of threads.
https://stackoverflow.com/questions/902425/does-multithreading-make-sense-for-io-bound-operations

A = Total number of files to process. Assume this can be infinite.
B = Average size of file. Not infinite. 10M or less.
C = Total number of words found. Size of the dictionary. 100k max ? Opimize for search before insertion.
D = number of hardware cores (assumed to be the natural best number of worker threads)
E = Serialized connections to the file system. (Assume 1 for normal local hard disk, more for NAS)

### To build on Windows: 

Use Visual Studio Community 2017.
Load the sln file.
Press F5

### To Run on Windows: 

ssfi c:\tmp -createsimpletest 1000 -quit
ssfi c:\tmp -t 7

### To build on Linux: (e.g. Ubuntu 18.04 LTS minimal install) 

cd ~/
sudo apt-get install gcc subversion make perl g++ build-essential g++-multilib
svn checkout --no-auth-cache https://home.menasoft.com:8443/svn/Public/CountTextWords CountTextWords --username public
cd CountTextWords
svn update --force 
rem ? install rt, pthread for : librt.so, pthread.so 
rem ? check include files in /usr/include/c++/7
make
 
rem svn cleanup
rem svn update

### To Run on Linux: 
./ssfi ~/tmp -createsimpletest 1000 -quit
./ssfi ~/tmp
./ssfi /usr/share/doc -t 7

### Possible improvements?: 

Precompiled header not working with makefile.

Doxygen

Chunk single files across multiple threads ? see above. More threads probably wont make it better.
Create private dictionaries for threads then merge at end of thread to avoid frequent locking/contention ?
cThreadFileReader::ReadFile might be faster with block reads not char reads ?

more logging

make a big index for sorting usage count/rank as words are added ? Not an improvement. Walking dictionary as last step optimizes A not C.

Let directory reader get a head start before it has contention with file readers ? Testing seems to indicate no.

is reading one char at a time via ifstream.read very slow or does underlying buffering make up for it ?

## Official Criticism

The code performed worse as the thread count increased.  
We identified many of the causes for the negative behavior in his code and the candidate should have found them.

(my take)
Since the disk device is likely serial (unless SAN, RAID etc) I would axpect it to perform worse with more threads past a certain amount. Maybe adding some serialization (locking ) to the disk would help? Thats going to hurt or help depending on the target.

### Versions 

v1
v2 = fix variables not initialized on Linux.
v3 = remove c++17 need and use of experimental path.
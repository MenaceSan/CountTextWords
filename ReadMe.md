# NetApp SSFI Interview application
AKA CountTextWords
By Dennis Robinson (dennis@menasoft.com)
2018-12-01
A NetApp interview C++ problem based on "Element Core Dev Coding Assignment.docx"

NOTE: This code FAILED review! (see criticism section below) I'd love to get feedback as to why. Any comments in the GitHub Issues area would be welcome. I'll try to find a better forum for this as well.

See:
https://stackoverflow.com/questions/53878291/if-i-need-to-read-lots-of-files-will-it-get-faster-if-i-break-the-problem-into


### Summary
Create an app that reads all '*.txt' files in a given directory.
Create one directory enumeration thread and N other threads to read the actual file contents.
The reader threads should count the number of occurrences of each word. Words are delimited by non alphanumerics.
When complete, print out a list of the top ten occurring words.

### Goals:

Fastest possible run time. Optimized for large number of files (A) preferred over other optimizations.<br />
Avoid slower interfaces. e.g. using shell for directory list.<br />
Keep it small and simple. Write Less code. Easy to read.<br />
Dev Time limited. Can only spend so much time on this.<br />
Portable to Windows and most Linux distros.<br />
-Help mode.<br />
Show run timing.<br />
Fail gracefully.<br />
Commented code.<br />
Use STL for all possible.<br />
Clean cancellation in windows.<br />
No build warnings<br />
use const keyword appropriately for best optimization.<br />

### Assumptions:

c++17 for std::filesystem support on MSVC. experimental for g++.

Most files will be serialized to a single device so threading might not be effective. 
Most operations will be i/O bound. SAN arrays may break that rule.
Number of cores on the machine = optimal number of threads.

https://stackoverflow.com/questions/902425/does-multithreading-make-sense-for-io-bound-operations

A = Total number of files to process. Assume this can be infinite.<br />
B = Average size of file. Not infinite. 10M or less.<br />
C = Total number of words found. Size of the dictionary. 100k max ? Opimize for search before insertion.<br />
D = number of hardware cores (assumed to be the natural best number of worker threads)<br />
E = Serialized connections to the file system. (Assume 1 for normal local hard disk, more for NAS)<br />

### To build on Windows: 

Use Visual Studio Community 2017.
Load the sln file.
Press F5.

### To Run on Windows: 

ssfi c:\tmp -createsimpletest 1000 -quit<br />
ssfi c:\tmp -t 7

### To build on Linux: (e.g. Ubuntu 18.04 LTS minimal install) 

cd ~/<br />
sudo apt-get install gcc subversion make perl g++ build-essential g++-multilib<br />
svn checkout --no-auth-cache https://home.menasoft.com:8443/svn/Public/CountTextWords CountTextWords --username public<br />
cd CountTextWords<br />
svn update --force <br />
rem ? install rt, pthread for : librt.so, pthread.so <br />
rem ? check include files in /usr/include/c++/7<br />
make
 
rem svn cleanup
rem svn update

### To Run on Linux: 
./ssfi ~/tmp -createsimpletest 1000 -quit<br />
./ssfi ~/tmp<br />
./ssfi /usr/share/doc -t 7<br />

### Possible improvements?: 

Precompiled header not working with makefile.

Doxygen

Chunk single files across multiple threads ? see above. More threads probably wont make it better.
Create private dictionaries for threads then merge at end of thread to avoid frequent locking/contention ?
cThreadFileReader::ReadFile might be faster with block reads not char reads ?

more logging

make a big index for sorting usage count/rank as words are added ? Not an improvement. Walking dictionary as last step optimizes A not C.

Let directory reader get a head start before it has contention with file readers ? Testing seems to indicate no.

mmap() might help if the files are large. But it will get slower if the files are small.


## Official Criticism

The code performed worse as the thread count increased.  
We identified many of the causes for the negative behavior in his code and the candidate should have found them.

(my take)
Since the disk device is likely serial (unless SAN, RAID etc) I would expect it to perform worse with more threads past a certain amount. Maybe adding some serialization (locking ) to the disk would help? Thats going to hurt or help depending on the target.

### Versions 

v1<br />
v2 = fix variables not initialized on Linux.<br />
v3 = remove c++17 need and use of experimental path.<br />

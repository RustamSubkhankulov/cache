### Overview
Implementation of LFU cache and its comparison with "perfect" cache algorithm - PCA as I named it. "Perfect" cache algorithm just basically knows future ^-^. Programm also performs simple test of cache by running test on sequence of int values from stdin or using predefined scenarios. 

### Cloning & Building
 - Firstly, copy source files or clone this repository: <code>git clone git@github.com:RustamSubkhankulov/cache.git</code>.
 - Secondly, change current working directory: 
 <code>cd cache</code>
 - Thirdly, build desired tests:
   - <code> cmake -B build </code>
   - <code> cmake --build build [--target <target_name>] </code>, where <code><target_name></code> is <b>'auto'</b> for auto-generated tests and <b>'interactive'</b> for interactive testing (see section <b>Testing</b> below).
   Omitting <code>--target</code> option will cause all targets to build. 
   Executables are located in <code>./build/bin</code> subdirectory.

#### Build options:
- DEBUG_FLAGS: use debug compilation flags for additional warnings. Turned off automatically on CMAKE_BUILD_TYPE=RELEASE

### Testing
Two testing options are supported:
 - <b>'interactive'</b> mode: awaits cache size, number of elements and elements from the user.
 - <b>'auto'</b> mode: generates and performs automatically several tests using pseudo-random values with rand().

#### How to use <b>'intreactive'</b>:
Programm behaves as following:
 - On stdin: cache size, number of elements, elements separated with whitespaces.
 - On stdout: cache hits number for every implemented caching algorithm: LFU and PCA

Here's an example:
- run command: <code>❯ ./build/bin/interactive> </code>
- enter cache size, number of elements and elements: <code>4 12 1 2 3 4 1 2 5 1 2 4 3 4</code>
- here's out results:
<div><code>Hits statistics:
 - LFU: 3
 - PCA (Perfect Caching Algorithm): 7</code></div>

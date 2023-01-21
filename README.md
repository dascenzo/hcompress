# hcompress
Command line program that compresses and decompresses files using Huffman coding.

## Building
To build, you need to be in a POSIX environment and have a C++17 compiler installed. From within the top-level directory of the project, run:

    $ make [BUILD=<debug|release>]

The resulting build, including `hcompress` binary, will be placed in the directory `build-<debug|release>/`. The default build type is `debug`.

## Example
    $ make BUILD=release
    $ cd build-release
    $ ./hcompress MyData.txt       # Compress file
    $ ls
    MyData.txt.hz                  # Compressed file has .hz extension
    $ ./hcompress -d MyData.txt.hz # Decompress file
    $ ls
    MyData.txt
The program has options to:
* Change the symbol byte length used for the Huffman coding.
* Keep or delete the original file.
* Print data pertaining to the compression, such as the space saving achieved.

Use the `-h` flag for usage information.

## Testing
There is a test script `test/test.sh`. It performs system testing by running the program with various input files and checking if the output is what's expected.

    $ ./test.sh -h
    usage: ./test.sh [-h] [-p|--show-passing] [-s|--stop-on-failure] <debug|release>

## License

See [LICENSE](LICENSE.txt).

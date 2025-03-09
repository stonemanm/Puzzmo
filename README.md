# Puzzmo Solver in C++/VSCode with Bazel and Abseil

Puzzles:

- [x] **Bongo**
  - [x] Try several permutations of highest-scoring tiles in multiplier spaces.
  - [ ] Somehow save daily checked words between runs.
  - [ ] Multiple solver types.
- [x] **Spelltower**
  - [x] Get a list of all possible playable words on the board, and their scores.
  - [x] `SolveGreedily`
  - [ ] `SolveOptimally`
    - [x] Get longest potentially-possible word using all stars.
    - [ ] Find a way to make that word playable, or else determine it to be impossible and try another word.
- [x] **Typeshift**
  - [x] Find sets of words that use every letter.
  - [x] Find the _fewest possible_ words to use every letter.

Technical Features:

- [x] Building C++ files using Bazel in Visual Studio Code
- [x] Google's [Abseil library](https://github.com/abseil/abseil-cpp)
- [x] [Google Test](https://github.com/google/googletest) for unit tests

## Prerequisite: Installing Bazel

This repo uses `Bazel` for building C++ files.
You can install Bazel using this [link](https://docs.bazel.build/versions/master/install.html).

## License

<img align="right" src="https://149753425.v2.pressablecdn.com/wp-content/uploads/2009/06/OSIApproved_100X125.png" alt="OSI approved license">

The class is licensed under the [MIT License](https://opensource.org/licenses/MIT):

Copyright &copy; 2013-2024 [Niels Lohmann](https://nlohmann.me)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

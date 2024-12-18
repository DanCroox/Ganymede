Ganymede is a multiple purpose game engine. It is a personal project to grow my knowledge about the operation principles of a modern game engine.

The initial commit within this repo contains a cleaned up version of the original code of the engine (which is not part of this repo).
The engine already contains quite a lot of features - however, the code in this repo is still going thru a huge refactor which is not ready yet.
Threfore the engine code is not ready to run properly with all features (or run at all) - for the sake of cleaner code.

The main reasons for the refactoring are:
- Use a proper build system
- Destinquish between the engine core code and client code (the engine code is a loose collection of multipurpose high-level tools, while the client provides a way to use them in a particular way)
- Abstract the renderer (to implement different render APIs later)
- Overall code clean-up

Once this is done, the main branch of this repo will not be stable or fully functional at all. Later i will introduce a second branch with the latest codes changes (+main with stable version of the engine).

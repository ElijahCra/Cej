add_executable(CejCompiler)


target_sources(CejCompiler
        PUBLIC
        main.cpp
)
target_sources(CejCompiler
        PUBLIC
        FILE_SET all_my_modules TYPE CXX_MODULES FILES
        Lexer.cpp
        Parser.cpp
        Generator.cpp
)
#add_subdirectory(Lexer)


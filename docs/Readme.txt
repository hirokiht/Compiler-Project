===============================
Name	: Ng Wen Jiet 黄文杰
Stu.ID	: F74017023
Email	: jiet5417@hotmail.com

Name	: 竹內宏輝
Stu.ID	: F74017081
Email	: hirokiht@gmail.com
===============================

- The code is in C++ standard, compiled and tested using g++ version 4.7.3 in Ubuntu 13.04
- All C++ compliment compiler should be supported, such as Visual Studio 2012


What we done.
----------------------
- Basic output 1 and output 2, output 3.

Files.
----------------------
Project contains 10 files: 	main.cpp
							parserGenerator.cpp
							parserGenerator.hpp
							SLRparser.cpp
							SLRparser.hpp
							semanticAnalyzer.cpp
							semanticAnalyzer.hpp
							grammarSLR.txt
							Readme.txt
							input.java



How to Build and run the program.
-------------------------
To compile in Unix-like environment:
	cd <path to the source code>
	g++ ./src/*.cpp

To compile in Windows environment:
	Obtain a compiler of your choice.
	For gcc, the command is same as above; for other compilers, kindly refer to the compiler documentation

To run:	<output file> <grammar file> <.java file>
	example: ./a.out ./grammar/grammarSLR.txt input.java
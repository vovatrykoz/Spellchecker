# General Info

A simple spellchecker written in C++.
CMake version 3.20 or later is required to build and run this project.

# Includes

I have added an include folder, which contains two header files: spellchecker.h and clustering.h. 
Spellchecker.h contains all the logic connected to spellchecking, including Levenshtein distance calculation, finding the most central word, etc.
Clustering.h contains all the functions that do the actual clustering.

# Project start-up

The program will now expect one argument before start-up, which is the path to a file containing all the words we want to include in the spellchecker. The file should be a .txt file, with one word per line.
I have added some txt files in the data folder. If you want to run and debug the program in an IDE or code editor, make sure you send the path as an argument (for example, through launch.json). 
The words that we came up with during our meeting are located under data/words.txt. There are also other .txt containing words if you want to play with them.

Once you've compiled the project, you can start it like this: 

``./Spellchecker <path-to-file-with-words>``

where ``<path-to-file-with-words>`` is the path to the .txt file containg all the words you want to include into the spellchecker, one word per line.

When you start it up, it will read the file, and split the words into clusters. You can then input words and see how well it corrects them. There are also special commands. Here is how the interface looks like

<img width="1095" height="574" alt="image" src="https://github.com/user-attachments/assets/7ebabbfb-a2aa-47c2-9da4-6c944ca4efa3" />


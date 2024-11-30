# General Info

I restructured the project. 
It does the same things that it used to do in the old version, but I tried to split it up into logical components and wrap common logic into functions. I have added a simple CLI to our program.

# Includes

I have added an include folder, which contains two header files: spellchecker.h and clustering.h. 
Spellchecker.h contains all the logic connected to spellchecking, including Levenshtein distance calculation, finding the most central word, etc.
Clustering.h contains all the functions that do the actual clustering.

# Project start-up

The program will now expect one argument before start-up, which is the path to a file containing all the words we want to include in the spellchecker. The file should be a .txt file, with one word per line
I have added some txt files in the data folder. If you want to run and debug the program in an IDE or code editor, make sure you send the path as an argument (for example, through launch.json). 
The words that we came up with during our meeting are located under data/words.txt. There are also other .txt containing words if you want to play with them.

Once you've compiled the project, you can start it like this: 

``./spellchekcer <path-to-file-with-words>``

where ``<path-to-file-with-words>`` is the path to the .txt file containg all the words you want to include into the spellchecker, one word per line.

When you start it up, it will read the file, and split the words into clusters. You can then input words and see how well it corrects them. There are also special commands. Here is how the interface looks like

![Skärmbild 2024-03-03 184118](https://github.com/phlindstedt/MAA507-Spell-checker/assets/88552890/c14f8b0d-10f3-4442-a7e0-1b1ba6fd09c8)

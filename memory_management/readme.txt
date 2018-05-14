default compiler would work.

step 1 - compile the code using make.

$ make

step 2 - run the code.

$ ./runit.sh . ./test-output ./main.out

step 3 - grade the code.

$ ./gradeit.sh refout test-output

Where ./refout is the reference output directory, and ./test-output is the output directory.
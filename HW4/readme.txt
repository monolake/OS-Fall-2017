1. run "make" to make the exe.
2. "mkdir test-output".
3. change runit.sh 

#SCHED=${*:-../src/iosched}
SCHED=${*:-./main.out}

4. generate output 

"./runit.sh ./test-output/"

5. grade it

"./gradeit.sh ./refout/ ./test-output/"

$ ./gradeit.sh ./refout/ ./test-output/
                 i j s c f
0                .  .  .  .  .
1                .  .  .  .  .
2                .  .  .  .  .
3                .  .  .  .  .
4                .  .  .  .  .
5                .  .  .  .  .
6                .  .  .  .  .
7                .  .  .  .  .
8                .  .  .  .  x
9                .  .  .  .  .
SUM             10 10 10 10  9
TOTAL: 49 out of 50

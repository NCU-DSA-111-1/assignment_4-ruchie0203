# Snake in C

Final project - connection part

# Compile
```
make:
    gcc server.o -o server -lpthread
    gcc snaklient.o -o snaklient -lpthread
```
# Run
For server:
```
    ./server
``` 
For client(player):
```
    ./snaklient
```
# Feature
1. The windows should be **full screen**

2. Use the arrow key to control the move direction of the yello snake.

3. Enjoy the classic game "snake". 

4. The client will send the message with direction message and the server send it to all clients(if there are multiple clients) to make the specific snake change the direction.



# Bugs
1. Can't change the direction properly while the sending and receiving part have no problem (I thought).

2. We now disable the input rule to make the snake be able to turn to all directions, so the whole program still need to be fixed and completed.

3. In this version, **you must enter any direction at the beginning** or the game immediately(the snake will crash into a invisible wall first...)

4. Be careful that in this version you can change the opposite direction *(e.g. turn to UP while going DOWN)* which will **END THE GAME**!

5. Sometimes the game will end by itself and I don't know why in the moment.
The program starts in `frogger.cpp` `main()` function.

# Main

Firstly, we need to call `stdio_init_all` from Pico SDK to initialize everything. Than we setup the GPIO to our needs. Most GPIO initialization is in function `setup_gpios` where is initialized everything except for buttons (they are initialized in constructor of `Button` class)

Than create `Button` object for start button.

Than we need to initialize the display that we are using (SSD1306) using the `pico_ssd1306` library.

Than it is time to show the starting screen and begin the main loop that is responsible for (re)starting the game itself using helper function `game_start`.

# Button

Button class is used to simplify working with buttons. There is only one function that can be called and that is `isPressed` which takes timestamp as an argument. So the caller of the function is responsible for getting the current time and passing it to the button. This is done in order to speed up the `isPressed` function because we are using multiple buttons that we check all the time and getting current time is quite slow.

# Game start
`game_start` function is responsible for loading all images, spawning of every object in the playing area and starting of the game loop.

Images are `static` variable which results in  loading them only once. They are not modified in any way during execution of the game so they could also be `const`. For each image we need to create also `Image` object that is taking care of remebering the dimensions of the image and drawing it on the screen.

In order to initialize the game engine, we need to firstly configure some options related to the player (button controls and frog properties). 

After initializing the game engine, we need to spawn all objects into the scene. because by starting of the game loop we give up the control of the program while the game is being played.

Spawning of objects is relatively simple because the `GameEngine` has helper methods for spawning of different kind of game objects. So, we only need to specify the position, the image and name of the object (4 letter name used during debugging). For objects that are moving automatically (platforms and cars), we also need to specify how the objects will be moving. It is given by `MotionVector` and step time. The object will be moved by the motion vector every step where one step is exactly what we specify.

After spawning everything we can start the game loop

# Game loop

The game loop is an infinite cycle (which ends by player death) with the following steps. Firstly it gets the current time for all updates of objects (including checking if buttons are pressed). After updating all objects, the display is cleared and new frame is drawn (every object in the scene will draw itself). Then we check if the game should end or not (`checkCollisions` checks if the game should end and displays the game over/victory message). We will display the current frame buffer and break the game loop if it is game over.

# Game engine

`GameEngine` class is responsible for keeping track of objects spawned in the scene and for keeping the game running and potentionally end the game when it should end. It is NOT responsible for restarting game when the previous one ended.

Part of this class is the main game loop which calls functions for object update and object draw.

# Game object hierarchy

## GameObject (base class)

`GameObject` class represents a non-moving object with certain image. The fact that it is non-moving means only that it does not move automatically. It can however be moved by setting coordinates x,y.

It has 2 virtual methods ,`updateTick` and `draw`, that can be reimplemented in derived classes. 

`updateTick` should be responsible for updating the position of the object but it does not have to. It is called every frame before drawing any object on screen. It has one argument and that is current time (game loop is responsible for getting the current time). All game objects receive the same time so it might not be precise. 

`draw` is responsible for drawing it self on display. The only argument of this method is pointer to display driver which is initialized in `main` function. 
Image can also be flipped by setting booleans in the `Image` struct. The flip is performed around the center. The flip is used in this game to  make the cars go in both directions with only one image.

## Physics objects

`PhysicsObject` is derived from GameObject and it reimplements `updateTick` and `draw`. This class is mainly for objects that are constantly moving and/or needs to be able detect collisions.

The movement is defined by `step_time_us` and `motion_vector`. A step is that the object moves by the motion vector (motion vector is added to x and y coordinates) and step is performed once every `step_time_us`. If `updateTick` is not called in time to perform a step, it performs all steps that were not performed. This behaviour can lead to missed collisions (if multiple steps are performed at once).

There is also a method `synchronize` which can be used to synchronizes last updates of two objects. It is mainly for situation when two objects should be moving together (frog is on moving platform). If it is not used, it can be out of sync (e.g frog moves first and after some time the platform moves). It has only one argument and that is the last update of the other objects that should move in sync.

If `loop_` variable is set, the object will wrap around if it is at the edge of the screen. Edges of the screen are the sides of rectangle with opposite corners (0, 0) (max_x_, max_y_).

Collisions can be checked using `colidesWithObjects` method. It is a template because vector of `shared_ptr<T>` cannot be passed using derived classes of T. Collisions are detected by checking if the bounding boxes intersect. It is checked for every object in vector passed. If it detects a collision the other object that it collided with is returned. However, if no collision is detected a `nullptr` is returned.

## Frog

`Frog` class is derived from `PhysicsObject`. It has a static variables for the Image. Because it has many parameters to set up, a struct is used (`frog_options_t`).

In options, there are pins for buttons (and debounce time) that are used to control the movement. It creates `Button` object for each pin passed. Currently only up, down, left, right buttons are used.

`updateTick` is reimplemented and is responsible for movement of the frog. It checks if any of the buttons are pressed and moves the frog accordingly. Also it calls PhysicsObjects's method `updateTick` so that the frog can move on moving platforms. This movement is set in the main gameloop when the game engine is checking for collisions.


## Protocol

The betaflight process communicates with the game over two UDP sockets.
One from the game to the betaflight process and one from the process to the game.
All messages are serialized using the Godot binary format 
(see [docs](https://docs.godotengine.org/en/3.1/tutorials/misc/binary_serialization_api.html)).

### Connection

The connection is started by the game which sends an init packet.
This init packet contains all relevant information to build the physics model of the drone.
After receiving the init packet, the process will send a response consisting of a boolean value back.
This way the game knows it has successfully established connection with the betaflight process.

After this a state packet is send from the game to the process every physics step.
The betaflight process will respond with an update packet containing the linear and angular velocity of the drone.
Every two updates an OSD update packet will be sent. This packet also contains an OSD buffer.
The OSD update is not done every frame as the physics loop runs faster that the graphics loop.

### Packets

The exact contents of the packets can be found [here](https://github.com/timower/KwadSimSITL/blob/master/src/packets.def).

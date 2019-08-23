Godot -> KwadSimServer
----------------------
init:
    Kv, R, I0 : float
    Rpm, a, torqueF, inertia : float
    thrustVel: Floatx3
    dragArea: Vector3, dragC: float
    Vbat: float

state:
 delta_time: float           4
 Transform:  Vec3 + Mat3x3   12+36
 angVel:     Vector3         12
 linearVel:  Vector3         12
 rcData:     8 * float       32
 crashed:    bool            1
 ---------------------------------
                             109 bytes

KwadSimServer -> Godot
----------------------

state:
 linVel: Vector3  12
 angVel: Vector3  12
----------------------
                 24

sometimes?:
 osd: 16*30
extends Object

class_name Packets

#define GEN # This file is generated from KwadSimSITL/Packets.def, do not edit
GEN

class Packet extends Object:
    var _props = []
    
    func _init():
        for prop in get_property_list():
            if prop.usage == PROPERTY_USAGE_SCRIPT_VARIABLE and prop.name != "_props":
                _props.append(prop.name)

    func to_list():
        var result = []
        result.resize(len(_props))
        for i in range(len(_props)):
            result[i] = get(_props[i])
            assert(result[i] != null, _props[i]+ " is null")
        return result
    
    func from_list(lst):
        for i in range(len(_props)):
            set(_props[i], lst[i])
            assert(get(_props[i]) != null,  _props[i]+ " set null")

#define PACKET(name, size)  \
class name extends Packet:

#define FIELD(type, name) var name: type

#define S(...) Array

#define FloatT float
#define Vec3T Vector3
#define BoolT bool
#define ArrayT Array
#define BasisT Basis
#define PoolByteArrayT PoolByteArray

#define END_PACKET()

#include "../src/packets.def"

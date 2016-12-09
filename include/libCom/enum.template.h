[[[cog
import cog
import enum_generator as g
import common as c

from os.path import basename, splitext
import math

name = splitext(basename(filename[:-4]))[0]
filename = base_path+filename
]]]
[[[end]]]
[[[cog
cog.outl("/*** AUTOMATICALLY GENERATED FILE - DO NOT EDIT! ***/")
]]]
[[[end]]]

[[[cog
cog.outl("#ifndef {name}_H".format(name=name))
cog.outl("#define {name}_H".format(name=name))
]]]
[[[end]]]

#include <string>
#include <iostream>

[[[cog
    vals = list(g.do(filename))
    val_bits = int(math.log(len(vals)))
    enum_type = c.bits_to_type(val_bits)

    cog.outl("enum class "+name+" : {type} {{".format(type=enum_type))

    # iterate over enum values
    for val in vals:
        id = val[0]
        comment = val[1]
        cog.outl("    {value},        {comment}".format(value=id, comment=comment))
]]]
[[[end]]]
};

[[[cog
    cog.outl("inline std::string toString(const {name} &e) {{".format(name=name))
    cog.outl("    switch (e) {")
    for val in vals:
        id = val[0]
        cog.outl('    case {name}::{value}: return "{name}::{value}";'.format(name=name, value=id))
    cog.outl('    }');
    cog.outl('    return "";');
    cog.outl("}");
]]]
[[[end]]]

[[[cog
    cog.outl("inline {type} toInt(const {name} &e) {{".format(type=enum_type, name=name))
    cog.outl('    return static_cast<{type}>(e);'.format(type=enum_type, name=name))
    cog.outl("}");
]]]
[[[end]]]

[[[cog
    cog.outl("inline {name} to{name}({type} val) {{".format(type=enum_type, name=name))
    cog.outl('    if (val > {max_val}) return {name}::{invalid_val};'.format(max_val=len(vals)-1, name=name, invalid_val=vals[-1][0]))
    cog.outl('    return static_cast<{name}>(val);'.format(type=enum_type, name=name))
    cog.outl("}");
]]]
[[[end]]]

[[[cog
cog.outl("inline std::ostream &operator<<(std::ostream &os, const {name} &e) {{".format(name=name))
cog.outl("    return (os << toString(e));")
cog.outl("}");
]]]
[[[end]]]

[[[cog
cog.outl("#endif //{name}_H".format(name=name))
]]]
[[[end]]]

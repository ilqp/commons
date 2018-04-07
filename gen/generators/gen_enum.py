import re
from os.path import basename, splitext
from common import CogBase, DefBase, read_definition, suggested_type

# matches
# "enum_value"
# "enum_value, comment"
line_matcher = re.compile(r"(?P<value>[a-zA-Z0-9_]*)\s*,?(?P<comment>[^#]+)?\s*(?:#.*)?")
# matches "import enum/path/to/EnumName.the"
import_matcher = re.compile(r"^import\s(?P<path>.+)$")


# a single enum element
class EnumElem(CogBase):
    def __init__(self, line):
        # match
        m = line_matcher.match(line)

        # extract
        self.value = m.group('value').strip()
        self.comment = m.group('comment')

        # check required fields
        if len(self.value) == 0:
            raise Exception("Parsing error in line: " + line)

        # strip optional field
        self.comment = "" if self.comment is None else self.comment.strip()


# all enum definitions
class EnumDef(DefBase, CogBase):
    def __init__(self, filename):
        DefBase.__init__(self, filename)

        # add invalid enum value
        self.body.append("INVALID_ENUM_VALUE,// invalid enum values are mapped to this")
        # create elements
        self.parse()

        # name enum after basename of the file
        self.name = splitext(basename(filename))[0]
        # use smallest type that can fit all elements
        self.type = suggested_type(len(self.elements))
        # include path for enum imports
        self.import_path = splitext(filename)[0] + ".h"

        self.max_val = len(self.elements) - 1
        self.invalid_val = self.elements[-1].value

    def parse_line(self, line):
        # enum definitions only have enum lines
        return [EnumElem(line)]


def enum_import(line):
    # match import
    m = import_matcher.match(line)
    if m is None:
        return None

    # extract path
    path = m.group('path').strip()

    # return EnumDef from path
    return EnumDef(path)

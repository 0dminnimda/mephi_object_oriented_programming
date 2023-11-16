import sys
import re
from pathlib import Path


# uml = Path(dict(enumerate(sys.argv)).get(1, "design.puml"))
uml = Path(sys.argv[1])

text = uml.read_text("utf-8")

text = text.replace("std::", "std++")
text = text.replace("::", "__")
text = text.replace("std++", "std::")
text = re.sub(text, r"^.+GDCLASS.+$", "")
text = text.replace("std++", "std::")

uml.write_text(text, "utf-8")

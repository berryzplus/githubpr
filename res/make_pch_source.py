# -*- coding:utf-8 -*-
import sys
import pathlib
args = sys.argv
preCompiledHeaderName = "StdAfx" if len(args) <= 1 else args[1]
preCompiledHeaderDir = "./" if len(args) <= 2 else args[2] + "/"
preCompiledHeaderSource = preCompiledHeaderName + ".cpp"
preCompiledHeaderPath = preCompiledHeaderDir + preCompiledHeaderSource
if pathlib.Path(preCompiledHeaderPath).exists():
    # do nothing
    sys.exit()
else:
    with open(preCompiledHeaderPath, "w") as f:
        print(f"// {preCompiledHeaderSource} : the Source-Code for Pre-Compiled Header", file=f)
        print(f"#include \"{preCompiledHeaderName}.h\"", file=f)

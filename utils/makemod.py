

from pathlib import Path
import os
import subprocess
import re

# global config, instance of class MakeModConfig
g_config = None

class MakeModConfig:
    def __init__(self) -> None:
        self.nemesis_program_name = "NemesisUnlimitedBehaviorEngine.exe"        # compiled executable name
        self.work_dir = Path.cwd()      # current working directory, should point to path to build dir containing compiled executable
        if "MSYSTEM" in os.environ :
            self.running_in_msys = True     # are we running inside msys2 shell?
            self.msys = os.environ["MSYSTEM"]
            self.msys_prefix = os.environ["MSYSTEM_PREFIX"]
        else:
            self.running_in_msys = False




def getDllsFromExecutable(config, filepath):
    dlls = { "SYSTEM" : [], "MSYS": []}
    run_args = ["ldd", filepath]
    cp = subprocess.run(run_args, capture_output=True, text=True)
    if cp.returncode == 0:
        dll_lines = re.split(r'\n', cp.stdout)
        pattern = re.compile(r'^\s*([\S]+)\s=>\s([\S]+)\s')
        pattern_systemdll = re.compile(r'/[a-zA-Z]/windows/system32/.+\.dll', flags=re.IGNORECASE)
        pattern_msysdll = re.compile(r'/'+config.msys+ r'/bin/.+\.dll', flags=re.IGNORECASE)
        for line in dll_lines:
            dll_match = pattern.match(line)
            if dll_match:
                dll_name = dll_match.group(1)
                dll_full_path = dll_match.group(2)
                if pattern_systemdll.match(dll_full_path):
                    # windows dll - do not copy to mod
                    dlls["SYSTEM"].append(dll_full_path)
                elif pattern_msysdll.match(dll_full_path):
                    #  dll from msys , we need this in same directory as executable
                    dlls["MSYS"].append(dll_full_path)
                else:
                    print("error: unable to match needed dll to category. dll=[",dll_full_path,']',sep='')

        return dlls
    else:
        return None


def main():
    global g_config
    g_config = MakeModConfig()

    print(g_config.work_dir)

    if g_config.running_in_msys :
        print("running in MSYS2 with prefix =", g_config.msys_prefix)

        print("dlls:", getDllsFromExecutable(g_config, g_config.nemesis_program_name))



if __name__ == '__main__':
    main()

